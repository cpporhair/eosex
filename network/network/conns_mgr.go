package network

import (
	"bytes"
	"encoding/hex"
	"eosex/network/config"
	"eosex/network/crypto"
	"fmt"
	"net"
	"sync"
	"time"
)

var connsOnce sync.Once
var connsMgr *ConnsMgr

type PossibleConnections byte

const (
	PossibleConnectionsNone      PossibleConnections = 0
	PossibleConnectionsProducers                     = 1 << 0
	PossibleConnectionsSpecified                     = 1 << 1
	PossibleConnectionsAny                           = 1 << 2
)

type keysPair struct {
	publicKey  crypto.PublicKey
	privateKey crypto.PrivateKey
}

type ConnsMgr struct {
	p2pAddr                    string
	maxClientCount             uint32
	maxNodesPerHost            uint32
	numClients                 uint32
	suppliedPeers              []string
	allowedPeers               []crypto.PublicKey
	privateKeys                []keysPair
	allowedConnections         PossibleConnections
	Connections                []*Connection
	syncMgr                    *SyncMgr
	dispatchMgr                *DispatchMgr
	peerAuthenticationInterval time.Duration
	networkVersionMatch        bool
	chainId                    Sha256Type
	nodeId                     Sha256Type
	userAgentName              string
	startedSessions            int32
	connLock                   sync.RWMutex
	exitChan                   chan struct{}
	wg                         *sync.WaitGroup
	localTxns                  []*NodeTransactionState
}

func GetConnsMgr() *ConnsMgr {

	connsOnce.Do(func() {
		cID, _ := hex.DecodeString(config.ChainId)
		connsMgr = &ConnsMgr{
			p2pAddr:                    config.P2PListenEndpoint,
			maxClientCount:             0,
			maxNodesPerHost:            1,
			numClients:                 0,
			syncMgr:                    NewSyncMgr(config.SyncFetchSpan),
			dispatchMgr:                NewDispatchMgr(),
			peerAuthenticationInterval: time.Second * 1,
			chainId:                    cID,
			nodeId:                     cID,
			userAgentName:              "eos test agent",
			startedSessions:            0,
			exitChan:                   make(chan struct{}),
			wg:                         &sync.WaitGroup{},
		}
		for _, allowed := range config.AllowedConnection {
			if allowed == "any" {
				connsMgr.allowedConnections |= PossibleConnectionsAny
			} else if allowed == "producers" {
				connsMgr.allowedConnections |= PossibleConnectionsProducers
			} else if allowed == "specified" {
				connsMgr.allowedConnections |= PossibleConnectionsSpecified
			} else if allowed == "none" {
				connsMgr.allowedConnections = PossibleConnectionsNone
			}
		}
		if len(config.PeerKey) > 0 {
			for _, pk := range config.PeerKey {
				publicKey, err := crypto.NewPublicKey(pk)
				if err != nil {
					continue
				}
				connsMgr.allowedPeers = append(connsMgr.allowedPeers, publicKey)
			}
		}
	})
	return connsMgr
}

func (m *ConnsMgr) Startup() {
	go m.startListenLoop()
	m.startMonitors()

	for _, seed_node := range config.SuppliedPeers {
		m.connectByEndpoint(seed_node)
	}
}

func (m *ConnsMgr) connectByEndpoint(host string) string {
	if m.findConn(host) != nil {
		return "already connected"
	}
	c := NewConnectionByEndpoint(host)
	if c == nil {
		return "cannot connect to peer:" + host
	}
	m.addConn(c)
	m.connectByConn(c)
	return "added connection"
}

func (m *ConnsMgr) connectByConn(c *Connection) {
	c.Connecting = true
	err := c.ReConnect()
	if err != nil {
		return
	}
	if m.startSession(c) {
		c.SendHandshake()
	}
}

func (m *ConnsMgr) startListenLoop() {

	listen, err := net.Listen("tcp", m.p2pAddr)
	if err != nil {
		fmt.Println("listen error:", err)
		return
	}
	m.wg.Add(1)
	defer func() {
		listen.Close()
		m.wg.Done()
	}()

	for {
		select {
		case <-m.exitChan:
			return

		default:
		}

		//listen.SetDeadline(time.Now().Add(timeout))

		conn, err := listen.Accept()
		if err != nil {
			fmt.Println("accept error:", err)
			continue
		}
		m.wg.Add(1)
		go func() {
			m.handleConnection(conn)
			m.wg.Done()
		}()
	}
}

func (m *ConnsMgr) handleConnection(conn net.Conn) {
	var visitors uint32 = 0
	var from_addr uint32 = 0
	paddr := conn.RemoteAddr().String()
	for _, c := range m.Connections {
		if c.Conn != nil {
			if len(c.PeerAddr) == 0 {
				visitors++
				if paddr == c.Conn.RemoteAddr().String() {
					from_addr++
				}
			}
		}
	}
	if m.numClients != visitors {
		//ilog ("checking max client, visitors = ${v} num clients ${n}",("v",visitors)("n",num_clients));
		m.numClients = visitors
	}
	if from_addr < m.maxNodesPerHost && (m.maxClientCount == 0 || m.numClients < m.maxClientCount) {
		m.numClients++
		c := NewConnection(conn)
		m.Connections = append(m.Connections, c)
		m.startSession(c)
	} else {
		if from_addr >= m.maxNodesPerHost {
			//fc_elog(logger, "Number of connections (${n}) from ${ra} exceeds limit", ("n", from_addr+1)("ra",paddr.to_string()));
		} else {
			//fc_elog(logger, "Error max_client_count ${m} exceeded", ( "m", max_client_count) );
		}
		conn.Close()
	}
}

func (m *ConnsMgr) startSession(c *Connection) bool {
	if c == nil || c.Conn == nil {
		return false
	}
	c.Start(m.exitChan, m.wg)
	m.startedSessions++
	return true
}

func (m *ConnsMgr) startMonitors() {
	go m.startConnTimer()
	go m.startTxnTimer()
}

func (m *ConnsMgr) startConnTimer() {
	ticker := time.NewTicker(30 * time.Second)
	for {
		select {
		case <-m.exitChan:
			return

		case <-ticker.C:
			m.connMonitor()
		}
	}
}

func (m *ConnsMgr) connMonitor() {
	discards := []*Connection{}
	for _, c := range m.Connections {
		if !c.IsOpened() {
			if len(c.PeerAddr) > 0 {
				m.connectByConn(c)
			} else {
				discards = append(discards, c)
			}
		}
	}

	for _, c := range discards {
		m.delConn(c)
	}
}

func (m *ConnsMgr) findConn(host string) *Connection {
	for _, c := range m.Connections {
		if c.PeerAddr == host {
			return c
		}
	}
	return nil
}

func (m *ConnsMgr) findConnIndex(conn *Connection) int {
	for i, c := range m.Connections {
		if conn == c {
			return i
		}
	}
	return -1
}

func (m *ConnsMgr) Close(c *Connection) {
	if len(c.PeerAddr) == 0 && c.IsOpened() {
		if m.numClients == 0 {
			//fc_wlog( logger, "num_clients already at 0");
		} else {
			m.numClients--
		}
	}
	c.Close()
}

func (m *ConnsMgr) addConn(conn *Connection) {
	m.Connections = append(m.Connections, conn)
}

func (m *ConnsMgr) delConn(conn *Connection) {
	for i, c := range m.Connections {
		if c == conn {
			m.Connections = append(m.Connections[:i], m.Connections[i+1:]...)
			break
		}
	}
}

func (m *ConnsMgr) startTxnTimer() {
	ticker := time.NewTicker(30 * time.Second)
	for {
		select {
		case <-m.exitChan:
			return

		case <-ticker.C:
			m.expireTxns()
		}
	}
}

func (m *ConnsMgr) expireTxns() {
	m.expireTxnsByTime(TimePointSec{time.Now()})
	bn := GetChainMgr().GetChainLibNum()
	m.expireTxnsByBlockNum(bn)
	for _, c := range m.Connections {
		c.ExpireTrxStateByTime(TimePointSec{time.Now()})
		c.ExpireTrxStateByBlockNum(bn)
		c.ExpireBlkStateByBlockNum(bn)
	}
}

func (m *ConnsMgr) Stop() {
	close(m.exitChan)
	m.wg.Wait()
}

func (m *ConnsMgr) getAuthenticationKey() crypto.PublicKey {
	if len(m.privateKeys) > 0 {
		return m.privateKeys[0].publicKey
	}
	publicKey := crypto.NewEmptyPublicKey()
	return *publicKey
}

type VerifierFunc func(c *Connection) bool

func (m *ConnsMgr) SendAll(msg NetMessage, verify VerifierFunc) {
	for _, c := range m.Connections {
		if c.Current() && verify(c) {
			c.Enqueue(msg)
		}
	}
}

func (m *ConnsMgr) FindLocalTxnsById(id Sha256Type) *NodeTransactionState {
	for i := 0; i < len(m.localTxns); i++ {
		if bytes.Equal(m.localTxns[i].Id, id) {
			return m.localTxns[i]
		}
	}
	return nil
}

func (m *ConnsMgr) ModifyLocalTxnsByBlockNum(nts *NodeTransactionState, ubn *UpdateBlockNum) {
	ubn.UpdateNodeTransactionState(nts)
}

func (m *ConnsMgr) ModifyLocalTxnsByInFlight(nts *NodeTransactionState, uif *UpdateInFlight) {
	uif.UpdateNodeTransactionState(nts)
}

func (m *ConnsMgr) expireTxnsByTime(upperTime TimePointSec) {
	kept := []*NodeTransactionState{}
	for _, nts := range m.localTxns {
		if nts.Expires.After(upperTime.Time) {
			kept = append(kept, nts)
		}
	}
	m.localTxns = kept
}

func (m *ConnsMgr) expireTxnsByBlockNum(num uint32) {
	kept := []*NodeTransactionState{}
	for _, nts := range m.localTxns {
		if nts.BlockNum > num {
			kept = append(kept, nts)
		}
	}
	m.localTxns = kept
}

func (m *ConnsMgr) findAllowedKey(key *crypto.PublicKey) bool {
	for _, allowed := range m.allowedPeers {
		if allowed.Equal(key) {
			return true
		}
	}
	return false
}

func (m *ConnsMgr) AuthenticatePeer(msg *HandshakeMessage) bool {
	if m.allowedConnections == PossibleConnectionsNone {
		return false
	}
	if m.allowedConnections == PossibleConnectionsAny {
		return true
	}
	if m.allowedConnections&(PossibleConnectionsProducers|PossibleConnectionsSpecified) != 0 {
		allowed := m.findAllowedKey(&msg.Key)
		privateKey := m.findPrivateKey(&msg.Key)
		foundProducerKey := false
		//producer_plugin* pp = app().find_plugin<producer_plugin>();
		//if(pp != nullptr)
		//found_producer_key = pp->is_producer_key(msg.key);
		if !allowed && privateKey == nil && !foundProducerKey {
			return false
		}
	}
	if time.Now().After(msg.Time.Time.Add(m.peerAuthenticationInterval)) {
		return false
	}
	if !msg.Signature.IsEmpty() && !msg.Token.IsEmpty() {
		th, _ := MarshalBinary(msg.Time)
		hash := Sh256Hash(th)
		if !bytes.Equal(msg.Token, hash) {
			return false
		}
		peerKey, err := msg.Signature.PublicKey(msg.Token)
		if err != nil {
			return false
		}
		if (m.allowedConnections&(PossibleConnectionsProducers|PossibleConnectionsSpecified) != 0) && !peerKey.Equal(&msg.Key) {
			return false
		}

	} else if m.allowedConnections&(PossibleConnectionsProducers|PossibleConnectionsSpecified) != 0 {
		return false
	}
	return true
}

func (m *ConnsMgr) findPrivateKey(publikey *crypto.PublicKey) *crypto.PrivateKey {
	for i := 0; i < len(m.privateKeys); i++ {
		if m.privateKeys[i].publicKey.Equal(publikey) {
			return &m.privateKeys[i].privateKey
		}
	}
	return nil
}

func (m *ConnsMgr) SignCompact(publikey *crypto.PublicKey, digest Sha256Type) *crypto.Signature {
	privKey := m.findPrivateKey(publikey)
	if privKey != nil {
		sign, err := privKey.Sign(digest)
		if err == nil {
			return sign
		}
	}
	//producer_plugin* pp = app().find_plugin<producer_plugin>();
	//if(pp != nullptr && pp->get_state() == abstract_plugin::started)
	//return pp->sign_compact(signer, digest);
	return crypto.NewEmptySignature()
}
