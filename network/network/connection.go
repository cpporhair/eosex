package network

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"encoding/json"
	"eosex/network/config"
	"eosex/network/utils"
	"errors"
	"fmt"
	"io"
	"net"
	"runtime"
	"sync"
	"time"
)

type PeerBlockState struct {
	Id            Sha256Type
	BlockNum      uint32
	IsKnown       bool
	IsNoticed     bool
	RequestedTime TimePoint
}

type UpdateKnownByPeer struct {
}

func (u *UpdateKnownByPeer) UpdatePeerBlockState(pbs *PeerBlockState) {
	pbs.IsKnown = true
}

func (u *UpdateKnownByPeer) UpdateTransactionState(ts *TransactionState) {
	ts.IsKnownByPeer = true
}

type UpdateRequestTime struct {
}

func (u *UpdateRequestTime) UpdatePeerBlockState(pbs *PeerBlockState) {
	pbs.RequestedTime = TimePoint{time.Now()}
}

func (u *UpdateRequestTime) UpdateTransactionState(ts *TransactionState) {
	ts.RequestedTime = TimePoint{time.Now()}
}

type NodeTransactionState struct {
	Id            Sha256Type
	Expires       TimePointSec
	PackedTxn     *PackedTransaction
	SerializedTxn []byte
	BlockNum      uint32
	TrueBlock     uint32
	Requests      uint16
}

type TransactionState struct {
	Id              Sha256Type
	IsKnownByPeer   bool
	IsNoticedToPeer bool
	BlockNum        uint32
	Expires         TimePointSec
	RequestedTime   TimePoint
}

type UpdateTxnExpiry struct {
	newExpiry TimePointSec
}

func (u *UpdateTxnExpiry) UpdateTransactionState(ts *TransactionState) {
	ts.Expires = u.newExpiry
}

type UpdateBlockNum struct {
	newBnum uint32
}

func (u *UpdateBlockNum) UpdateNodeTransactionState(nts *NodeTransactionState) {
	if nts.Requests > 0 {
		nts.TrueBlock = u.newBnum
	} else {
		nts.BlockNum = u.newBnum
	}
}

func (u *UpdateBlockNum) UpdateTransactionState(ts *TransactionState) {
	ts.BlockNum = u.newBnum
}

func (u *UpdateBlockNum) UpdatePeerBlockState(pbs *PeerBlockState) {
	pbs.BlockNum = u.newBnum
}

type UpdateInFlight struct {
	incr int16
}

func (u *UpdateInFlight) UpdateNodeTransactionState(nts *NodeTransactionState) {
	nts.Expires.Add(time.Second * time.Duration(u.incr*60))
	if nts.Requests == 0 {
		nts.TrueBlock = nts.BlockNum
		nts.BlockNum = 0
	}
	nts.Requests = uint16(int16(nts.Requests) + u.incr)
	if nts.Requests == 0 {
		nts.BlockNum = nts.TrueBlock
	}
}

type SyncState struct {
	StartBlock uint32
	EndBlock   uint32
	Last       uint32
	StartTime  TimePoint
}

type QueueWrite struct {
	Buff []byte
	//callback
}

type Connection struct {
	BlkState             []*PeerBlockState
	TrxState             []*TransactionState
	PeerRequested        *SyncState
	Conn                 net.Conn
	OutstandingReadBytes uint
	BlkBuffer            []byte
	NodeID               Sha256Type
	LastHandshakeRecv    *HandshakeMessage
	LastHandshakeSent    *HandshakeMessage
	SentHandshakeCount   uint16
	Connecting           bool
	Syncing              bool
	ProtocolVersion      uint16
	PeerAddr             string
	NoRetry              GoAwayReason
	ForkHead             Sha256Type
	ForkHeadNum          uint32
	LastReq              *RequestMessage
	Org                  Tstamp //!< originate timestamp
	Rec                  Tstamp //!< receive timestamp
	Dst                  Tstamp //!< destination timestamp
	Xmt                  Tstamp //!< transmit timestamp
	Offset               float64
	TsBufferSize         uint32 //32
	Ts                   []byte
	closeChan            chan struct{}
	msgRecvChan          chan Message
	msgSendChan          chan NetMessage
	isOpened             bool
	lock                 sync.RWMutex
}

func NewConnectionByEndpoint(endpoint string) *Connection {
	conn, err := net.Dial("tcp", endpoint)
	if err != nil {
		fmt.Println("net.Dial error, ", err)
		return nil
	}
	return &Connection{
		Conn:                 conn,
		OutstandingReadBytes: 0,
		LastHandshakeRecv:    new(HandshakeMessage),
		LastHandshakeSent:    new(HandshakeMessage),
		PeerAddr:             endpoint,
		TsBufferSize:         32,
		Ts:                   make([]byte, 32),
		closeChan:            make(chan struct{}),
		msgRecvChan:          make(chan Message, 1000),
		msgSendChan:          make(chan NetMessage, 1000),
		isOpened:             true,
	}
}

func NewConnection(conn net.Conn) *Connection {
	return &Connection{
		Conn:                 conn,
		OutstandingReadBytes: 0,
		LastHandshakeRecv:    new(HandshakeMessage),
		LastHandshakeSent:    new(HandshakeMessage),
		TsBufferSize:         32,
		Ts:                   make([]byte, 32),
		closeChan:            make(chan struct{}),
		msgRecvChan:          make(chan Message, 1000),
		msgSendChan:          make(chan NetMessage, 1000),
		isOpened:             true,
	}
}

func (c *Connection) Connected() bool {
	return (c.Conn != nil && c.IsOpened() && !c.Connecting)
}

func (c *Connection) ReConnect() error {
	c.lock.Lock()
	defer c.lock.Unlock()

	if c.isOpened {
		return nil
	}
	if len(c.PeerAddr) == 0 {
		return errors.New("peer-addr is empty")
	}
	conn, err := net.Dial("tcp", c.PeerAddr)
	if err != nil {
		return err
	}
	c.Conn = conn
	c.closeChan = make(chan struct{})
	c.msgRecvChan = make(chan Message, 1000)
	c.msgSendChan = make(chan NetMessage, 1000)
	c.isOpened = true
	return nil
}

func (c *Connection) Current() bool {
	return (c.Connected() && !c.Syncing)
}

func (c *Connection) reset() {
	c.PeerRequested = nil
	c.BlkState = []*PeerBlockState{}
	c.TrxState = []*TransactionState{}
}

func (c *Connection) FlushQueues() {

}

func (c *Connection) IsOpened() bool {
	c.lock.RLock()
	defer c.lock.RUnlock()

	return c.isOpened
}

func (c *Connection) Close() {
	c.lock.Lock()
	defer c.lock.Unlock()
	if !c.isOpened {
		return
	}

	c.isOpened = false
	c.Conn.Close()
	c.Conn = nil
	c.Connecting = false
	c.Syncing = false
	c.reset()
	c.SentHandshakeCount = 0
	c.LastHandshakeRecv = &HandshakeMessage{}
	c.LastHandshakeSent = &HandshakeMessage{}
	GetConnsMgr().syncMgr.resetLibNum(c)
	close(c.closeChan)
	close(c.msgSendChan)
	close(c.msgRecvChan)
}

func (c *Connection) PeerName() string {
	if len(c.LastHandshakeRecv.P2PAddress) != 0 {
		return c.LastHandshakeRecv.P2PAddress
	}
	if len(c.PeerAddr) > 0 {
		return c.PeerAddr
	}
	return "connecting client"
}

type LoopFun func(exitChan chan struct{})

func startLoop(f LoopFun, exitChan chan struct{}, wg *sync.WaitGroup) {
	wg.Add(1)
	go func() {
		f(exitChan)
		wg.Done()
	}()
}

func (c *Connection) Start(exitChan chan struct{}, wg *sync.WaitGroup) {
	startLoop(c.readMessageLoop, exitChan, wg)
	startLoop(c.handleMessageLoop, exitChan, wg)
	startLoop(c.writeMessageLoop, exitChan, wg)
}

func (c *Connection) readMessageLoop(exitChan chan struct{}) {
	defer func() {
		recover()
		c.Close()
	}()

	for {
		select {
		case <-exitChan:
			return

		case <-c.closeChan:
			return

		default:
		}

		msg, err := c.readMessage()
		if err != nil {
			return
		}

		c.msgRecvChan <- msg
	}
}

func (c *Connection) readMessage() (Message, error) {
	data := make([]byte, 0)
	lengthBytes := make([]byte, 4, 4)
	if _, err := io.ReadFull(c.Conn, lengthBytes); err != nil {
		return nil, err
	}
	data = append(data, lengthBytes...)
	size := binary.LittleEndian.Uint32(lengthBytes)
	if size > 1024*1024 {
		return nil, errors.New("overrange size of message")
	}
	payloadBytes := make([]byte, size, size)
	count, err := io.ReadFull(c.Conn, payloadBytes)
	if err != nil {
		fmt.Println("ReadFull, error: ", err)
		return nil, err
	}
	if count != int(size) {
		err = fmt.Errorf("ReadFull read error, read[%d] expected[%d]", count, size)
		return nil, err
	}
	data = append(data, payloadBytes...)
	msg := NewMessageData(data)
	return msg, nil
}

func (c *Connection) handleMessageLoop(exitChan chan struct{}) {
	defer func() {
		recover()
		c.Close()
	}()

	for {
		select {
		case <-exitChan:
			return

		case <-c.closeChan:
			return

		case msg := <-c.msgRecvChan:
			if !c.IsOpened() {
				return
			}
			c.parseRecvMessage(msg)
		}
	}
}

func (c *Connection) parseRecvMessage(msg Message) {
	msgData := msg.(*MessageData)
	data := msgData.Serialize()
	msgInfo := &NetMessageInfo{}
	unpacker := NewUnpacker(data)
	err := unpacker.Unpack(msgInfo)
	if err != nil {
		fmt.Println("Unpack error: ", err)
		return
	}
	msgInfo.Raw = data
	c.handleRecvMessage(msgInfo)
}

func (c *Connection) handleRecvMessage(msgInfo *NetMessageInfo) {
	data, err := json.Marshal(msgInfo)
	if err != nil {
		fmt.Println("Marshal err: ", err)
	}
	fmt.Println("handle message:", string(data))

	netMsg := msgInfo.NetMsg

	localData := GetLocalDataInstance()
	switch netMsg.GetType() {
	case HandshakeMessageType:
		msg, ok := netMsg.(*HandshakeMessage)
		if !ok {
			fmt.Println("error")
		}

		c.handleHandshakeMessage(msg)
	case SignedBlockType:
		msg, ok := netMsg.(*SignedBlock)
		if !ok {
			fmt.Println("error")
		}
		blockNum := msg.BlockNumber()
		blockId, err := msg.BlockID()
		if err != nil {
			fmt.Println("blockId:", hex.EncodeToString(blockId))
			return
		}
		blockTime := msg.Timestamp
		fmt.Println("blockNum:", blockNum)
		//fmt.Println("blockId:", hex.EncodeToString(blockId))
		//bn := BlockNum(hex.EncodeToString(blockId))
		//fmt.Println("bn:", bn)

		//localData.SetLocalData(syncHeadBlock, syncHeadBlock, blockId, blockId, blockTime)
		localData.SetHeadData(blockNum, blockId, blockTime)

		c.handleSignedBlock(msg)
	case NoticeMessageType:
		msg, ok := netMsg.(*NoticeMessage)
		if !ok {
			fmt.Println("error")
		}
		c.handleNoticeMessage(msg)
	case TimeMessageType:
		msg, ok := netMsg.(*TimeMessage)
		if !ok {
			fmt.Println("error")
		}
		//strTime := msg.Transmit.Time.Format("2016-01-02 15:04:05")
		strTime := fmt.Sprintf("%q", msg.Transmit.Format(TstampFormat))
		strTime1 := msg.Transmit.Format(TstampFormat)
		fmt.Println(strTime, strTime1)
	case GoAwayMessageType:
		msg, ok := netMsg.(*GoAwayMessage)
		if !ok {
			fmt.Println("error")
		}
		c.handleGoAwayMessage(msg)
	case RequestMessageType:
		msg, ok := netMsg.(*RequestMessage)
		if !ok {
			fmt.Println("error")
		}
		c.handleRequestMessage(msg)
	default:
		fmt.Println("msg type:", netMsg.GetType())
	}
}

func ToProtocolVersion(v uint16) uint16 {
	if v > config.NetVersionBase {
		v -= config.NetVersionBase
		if v > config.NetVersionRange {
			v = 0
		}
		return v
	}
	return 0
}

func (c *Connection) isValid(msg *HandshakeMessage) bool {
	if msg.LastIrreversibleBlockNum > msg.HeadNum {
		return false
	}
	if len(msg.P2PAddress) == 0 {
		return false
	}
	if len(msg.OS) == 0 {
		return false
	}
	th, _ := MarshalBinary(msg.Time)
	token := Sh256Hash(th)
	if (!msg.Signature.IsEmpty() || !msg.Token.IsEmpty()) && !bytes.Equal(msg.Token, token) {
		return false
	}
	return true
}

func (c *Connection) handleHandshakeMessage(msg *HandshakeMessage) {
	if !c.isValid(msg) {
		c.Enqueue(NewGoAwayMessage(GoAwayFatalOther))
		return
	}
	libNum := GetChainMgr().GetChainLibNum()
	peerLib := msg.LastIrreversibleBlockNum
	if c.Connecting {
		c.Connecting = false
	}
	if msg.Generation == 1 {
		if bytes.Equal(msg.NodeID, GetConnsMgr().nodeId) {
			c.Enqueue(NewGoAwayMessage(GoAwaySelf))
			return
		}
		if len(c.PeerAddr) == 0 || c.LastHandshakeRecv.NodeID.IsEmpty() {
			for _, check := range GetConnsMgr().Connections {
				if check == c {
					continue
				}
				if check.Connected() && check.PeerName() == msg.P2PAddress {
					if !msg.Time.Add(c.LastHandshakeSent.Time).Time.After(check.LastHandshakeSent.Time.Add(check.LastHandshakeRecv.Time).Time) {
						continue
					}
					gam := NewGoAwayMessage(GoAwayDuplicate)
					gam.NodeID = GetConnsMgr().nodeId
					c.Enqueue(gam)
					c.NoRetry = GoAwayDuplicate
					return
				}
			}
		}
		if !bytes.Equal(msg.ChainID, GetConnsMgr().chainId) {
			c.Enqueue(NewGoAwayMessage(GoAwayWrongChain))
			return
		}
		c.ProtocolVersion = ToProtocolVersion(msg.NetworkVersion)
		if c.ProtocolVersion != config.NetVersion {
			if GetConnsMgr().networkVersionMatch {
				c.Enqueue(NewGoAwayMessage(GoAwayWrongVersion))
				return
			}
		}
		if !bytes.Equal(c.NodeID, msg.NodeID) {
			c.NodeID = msg.NodeID
		}
		if !GetConnsMgr().AuthenticatePeer(msg) {
			c.Enqueue(NewGoAwayMessage(GoAwayAuthentication))
			return
		}
		onFork := false
		if peerLib <= libNum && peerLib > 0 {
			peerLibId := GetChainMgr().GetChainBlockIdForNum(peerLib)
			if !msg.LastIrreversibleBlockID.Equal(peerLibId) {
				//.......
				//onFork = true
			}
			if onFork {
				c.Enqueue(NewGoAwayMessage(GoAwayForked))
			}
		}
		if c.SentHandshakeCount == 0 {
			c.SendHandshake()
		}
	}
	c.LastHandshakeRecv = msg
	GetConnsMgr().syncMgr.recvHandshake(c, msg)
}

func (c *Connection) handleGoAwayMessage(msg *GoAwayMessage) {
	c.NoRetry = msg.Reason
	if msg.Reason == GoAwayDuplicate {
		c.NodeID = msg.NodeID
	}
	c.FlushQueues()
	GetConnsMgr().Close(c)
}

func (c *Connection) handleNoticeMessage(msg *NoticeMessage) {
	c.Connecting = false
	req := &RequestMessage{}
	send_req := false
	switch msg.KnownTrx.Mode {
	case None:
	case LastIrrCatchUp:
		c.LastHandshakeRecv.HeadNum = msg.KnownTrx.Pending
		req.ReqTrx.Mode = None
	case CatchUp:
		if msg.KnownTrx.Pending > 0 {
			req.ReqTrx.Mode = CatchUp
			send_req = true
			knownSum := len(GetConnsMgr().localTxns)
			if knownSum > 0 {
				for _, nts := range GetConnsMgr().localTxns {
					req.ReqTrx.IDs = append(req.ReqTrx.IDs, nts.Id)
				}
			}
		}
	case Normal:
		GetConnsMgr().dispatchMgr.RecvNotice(c, msg, false)
	}

	switch msg.KnownBlocks.Mode {
	case None:
		if msg.KnownTrx.Mode != Normal {
			return
		}
	case LastIrrCatchUp, CatchUp:
		GetConnsMgr().syncMgr.recvNotice(c, msg)
	case Normal:
		GetConnsMgr().dispatchMgr.RecvNotice(c, msg, false)
	}
	if send_req {
		c.Enqueue(req)
	}
}

func (c *Connection) handleRequestMessage(msg *RequestMessage) {
	switch msg.ReqBlocks.Mode {
	case CatchUp:
		c.blkSendBranch()
	case Normal:
		c.blkSend(msg.ReqBlocks.IDs)
	}

	switch msg.ReqTrx.Mode {
	case CatchUp:
		c.txnSendPending(msg.ReqTrx.IDs)
	case Normal:
		c.txnSend(msg.ReqTrx.IDs)
	case None:
		if msg.ReqBlocks.Mode == None {
			c.stopSend()
		}
	}
}

func (c *Connection) handleSyncRequestMessage(msg *SyncRequestMessage) {
	if msg.EndBlock == 0 {
		c.PeerRequested = nil
		c.FlushQueues()
	} else {
		c.PeerRequested = &SyncState{msg.StartBlock, msg.EndBlock, msg.StartBlock - 1, TimePoint{time.Now()}}
		c.enqueueSyncBlock()
	}
}

func (c *Connection) handleSPackedTransaction(msg *PackedTransaction) {
	if GetConnsMgr().syncMgr.IsActive(c) {
		return
	}
	tid := msg.ID()
	if GetConnsMgr().FindLocalTxnsById(tid) != nil {
		return
	}
	GetConnsMgr().dispatchMgr.recvTransaction(c, tid)
	at := GetChainMgr().AcceptTransaction(msg)
	except := false
	if at != 0 {
		except = true
	}
	if !except {
		GetConnsMgr().dispatchMgr.bcastTransaction(msg)
		return
	}
	GetConnsMgr().dispatchMgr.rejectedTransaction(tid)
}

func (c *Connection) handleSignedBlock(msg *SignedBlock) {
	blkId, _ := msg.BlockID()
	blkNum := msg.BlockNumber()
	if GetChainMgr().GetBlockById(blkId) != nil {
		GetConnsMgr().syncMgr.recvBlock(c, blkId, blkNum)
		return
	}

	GetConnsMgr().dispatchMgr.recvBlock(c, blkId, blkNum)

	ab := GetChainMgr().AcceptBlock(msg)

	reason := GoAwayFatalOther
	switch ab {
	case 0:
		reason = GoAwayNoReason
	case 1:
	case 2:

	}

	ubn := &UpdateBlockNum{blkNum}

	if reason == GoAwayNoReason {
		for i := 0; i < len(msg.Transactions); i++ {
			var id Sha256Type
			if len(msg.Transactions[i].Transaction.ID) > 0 {
				id = msg.Transactions[i].Transaction.ID
			} else if msg.Transactions[i].Transaction.Packed != nil {
				id = msg.Transactions[i].Transaction.Packed.ID()
			} else {
				continue
			}
			ltx := GetConnsMgr().FindLocalTxnsById(id)
			if ltx != nil {
				GetConnsMgr().ModifyLocalTxnsByBlockNum(ltx, ubn)
			}
			ctx := c.FindTrxStateById(id)
			if ctx != nil {
				c.ModifyTrxStateByBLockNum(ctx, ubn)
			}
		}
		GetConnsMgr().syncMgr.recvBlock(c, blkId, blkNum)
	} else {
		GetConnsMgr().syncMgr.rejectedBlock(c, blkNum)
	}
}

func (c *Connection) writeMessageLoop(exitChan chan struct{}) {
	defer func() {
		recover()
		c.Close()
	}()

	for {
		select {
		case <-exitChan:
			return

		case <-c.closeChan:
			return

		case msg := <-c.msgSendChan:
			if !c.IsOpened() {
				return
			}
			err := c.handleSendMessage(msg)
			if err != nil {
				return
			}
		}
	}
}

func (c *Connection) handleSendMessage(msg NetMessage) error {
	if msg.GetType() == GoAwayMessageType {
		gam, ok := msg.(*GoAwayMessage)
		if !ok {
			return nil
		}
		if gam.Reason != GoAwayNoReason {
			defer GetConnsMgr().Close(c)
		}
	}
	msgInfo := &NetMessageInfo{
		Type:   msg.GetType(),
		NetMsg: msg,
	}
	//packer := NewPacker(c.Conn)
	//packer.Pack(msgInfo)
	buf := new(bytes.Buffer)
	paker := NewPacker(buf)
	err := paker.Pack(msgInfo)
	if err != nil {
		return nil
	}
	if _, err := c.Conn.Write(buf.Bytes()); err != nil {
		return err
	}
	return nil
}

func (c *Connection) blkSendBranch() {
	headNum := GetChainMgr().GetChainHeadBlockNum()
	note := &NoticeMessage{}
	note.KnownBlocks.Mode = Normal
	note.KnownBlocks.Pending = 0
	if headNum == 0 {
		c.Enqueue(note)
		return
	}
	libId := GetChainMgr().GetChainLibId()
	headId := GetChainMgr().GetChainHeadBlockId()
	bstack := []*SignedBlock{}
	for bid := headId; !bid.IsEmpty() && !bid.Equal(libId); {
		b := GetChainMgr().GetBlockById(bid)
		if b != nil {
			bid = b.Previous
			bstack = append(bstack, b)
		} else {
			break
		}
	}
	if len(bstack) > 0 {
		if bstack[len(bstack)-1].Previous.Equal(libId) {
			for _, bs := range bstack {
				c.Enqueue(bs)
			}
		}
	}
	c.Syncing = false
}

func (c *Connection) blkSend(ids []Sha256Type) {
	for i := 0; i < len(ids); i++ {
		b := GetChainMgr().GetBlockById(ids[i])
		if b != nil {
			bnum := b.BlockNumber()
			sendWhole := bnum <= GetChainMgr().GetChainLibNum()
			if sendWhole {
				c.Enqueue(b)
			} else {
				c.Enqueue(b)
			}
		} else {
			break
		}
	}
}

func (c *Connection) txnSendPending(ids []Sha256Type) {
	for _, tx := range GetConnsMgr().localTxns {
		if len(tx.SerializedTxn) > 0 && tx.BlockNum == 0 {
			found := false
			for i := 0; i < len(ids); i++ {
				if ids[i].Equal(tx.Id) {
					found = true
					break
				}
			}
			if !found {
				//uif := &UpdateInFlight{1}
				//GetConnsMgr().ModifyLocalTxnsByInFlight(tx, uif)
				pt := &PackedTransaction{}
				unpacker := NewUnpacker(tx.SerializedTxn)
				err := unpacker.Unpack(pt)
				if err != nil {
					fmt.Println("Unpack error: ", err)
					continue
				}
				c.Enqueue(pt)
			}
		}
	}
}

func (c *Connection) txnSend(ids []Sha256Type) {
	for i := 0; i < len(ids); i++ {
		tx := GetConnsMgr().FindLocalTxnsById(ids[i])
		if tx != nil && len(tx.SerializedTxn) > 0 {
			//uif := &UpdateInFlight{1}
			//GetConnsMgr().ModifyLocalTxnsByInFlight(tx, uif)
			//net_plugin.cpp:864
			pt := &PackedTransaction{}
			unpacker := NewUnpacker(tx.SerializedTxn)
			err := unpacker.Unpack(pt)
			if err != nil {
				fmt.Println("Unpack error: ", err)
				continue
			}
			c.Enqueue(pt)
		}
	}
}

func (c *Connection) stopSend() {
	c.Syncing = false
}

func (c *Connection) enqueueSyncBlock() bool {
	if c.PeerRequested == nil {
		return false
	}
	c.PeerRequested.Last++
	num := c.PeerRequested.Last
	//triggerSend := num == c.PeerRequested.StartBlock
	if num == c.PeerRequested.EndBlock {
		c.PeerRequested = nil
	}
	signedBlock := GetChainMgr().GetBlockByNumber(num)
	if signedBlock != nil {
		c.Enqueue(signedBlock)
		return true
	}
	return false
}

func (c *Connection) SendHandshake() {
	c.handshakePopulate(c.LastHandshakeSent)
	c.SentHandshakeCount++
	c.LastHandshakeSent.Generation = c.SentHandshakeCount
	c.Enqueue(c.LastHandshakeSent)
}

func (c *Connection) handshakePopulate(hello *HandshakeMessage) {
	tstamp := Tstamp{Time: GetChainMgr().GetChainHeadBlockTime().Time}

	hello.NetworkVersion = config.NetVersionBase + config.NetVersion
	hello.ChainID = GetConnsMgr().chainId
	hello.NodeID = GetConnsMgr().nodeId
	hello.Key = GetConnsMgr().getAuthenticationKey()
	hello.Time = tstamp
	th, _ := MarshalBinary(tstamp)
	hello.Token = Sh256Hash(th)
	hello.Signature = *GetConnsMgr().SignCompact(&hello.Key, hello.Token)
	if hello.Signature.IsEmpty() {
		hello.Token = NewEmptySha256Type()
	}

	hello.P2PAddress = GetConnsMgr().p2pAddr + " - " + utils.SubstrWithLength(hello.NodeID.String(), 0, 7)
	hello.OS = runtime.GOOS
	hello.Agent = GetConnsMgr().userAgentName
	//get from db
	hello.HeadNum = GetChainMgr().GetChainHeadBlockNum()
	hello.HeadID = GetChainMgr().GetChainHeadBlockId()
	hello.LastIrreversibleBlockNum = GetChainMgr().GetChainLibNum()
	hello.LastIrreversibleBlockID = GetChainMgr().GetChainLibId()
}

func (c *Connection) RequestSyncBlocks(startBlockNum uint32, endBlockNumber uint32) {
	fmt.Printf("RequestSyncBlocks start [%d] end [%d]\n", startBlockNum, endBlockNumber)
	syncRequestMsg := &SyncRequestMessage{
		StartBlock: startBlockNum,
		EndBlock:   endBlockNumber,
	}
	c.Enqueue(syncRequestMsg)
}

func (c *Connection) sendMessage(netMsg NetMessage) error {

	msgInfo := &NetMessageInfo{
		Type:   netMsg.GetType(),
		NetMsg: netMsg,
	}
	encoder := NewPacker(c.Conn)
	err := encoder.Pack(msgInfo)
	return err
}

func (c *Connection) Enqueue(netMsg NetMessage) {
	c.msgSendChan <- netMsg
}

func (c *Connection) AddPeerBlock(entry *PeerBlockState) bool {
	pbs := c.FindPeerBlockStateById(entry.Id)
	added := false
	if pbs == nil {
		added = true
		c.BlkState = append(c.BlkState, entry)
	} else {
		ukp := &UpdateKnownByPeer{}
		c.ModifyBlockStateByUpdateKnown(pbs, ukp)
		if entry.BlockNum == 0 {
			ubn := &UpdateBlockNum{entry.BlockNum}
			c.ModifyBlockStateByBLockNum(pbs, ubn)
		} else {
			urt := &UpdateRequestTime{}
			c.ModifyBlockStateByUpdateRequestTime(pbs, urt)
		}
	}
	return added
}

func (c *Connection) FindTrxStateById(id Sha256Type) *TransactionState {
	for i := 0; i < len(c.TrxState); i++ {
		if bytes.Equal(c.TrxState[i].Id, id) {
			return c.TrxState[i]
		}
	}
	return nil
}

func (c *Connection) ModifyTrxStateByBLockNum(ts *TransactionState, ubn *UpdateBlockNum) {
	ubn.UpdateTransactionState(ts)
}

func (c *Connection) ModifyTrxStateByTxnExpiry(ts *TransactionState, ubn *UpdateTxnExpiry) {
	ubn.UpdateTransactionState(ts)
}

func (c *Connection) ExpireTrxStateByTime(upperTime TimePointSec) {
	kept := []*TransactionState{}
	for _, ts := range c.TrxState {
		if ts.Expires.After(upperTime.Time) {
			kept = append(kept, ts)
		}
	}
	c.TrxState = kept
}

func (c *Connection) ExpireTrxStateByBlockNum(num uint32) {
	kept := []*TransactionState{}
	for _, ts := range c.TrxState {
		if ts.BlockNum > num {
			kept = append(kept, ts)
		}
	}
	c.TrxState = kept
}

func (c *Connection) FindPeerBlockStateById(id Sha256Type) *PeerBlockState {
	for i := 0; i < len(c.BlkState); i++ {
		if bytes.Equal(c.BlkState[i].Id, id) {
			return c.BlkState[i]
		}
	}
	return nil
}

func (c *Connection) ModifyBlockStateByUpdateKnown(pbs *PeerBlockState, ukp *UpdateKnownByPeer) {
	ukp.UpdatePeerBlockState(pbs)
}

func (c *Connection) ModifyBlockStateByUpdateRequestTime(pbs *PeerBlockState, urt *UpdateRequestTime) {
	urt.UpdatePeerBlockState(pbs)
}

func (c *Connection) ModifyBlockStateByBLockNum(pbs *PeerBlockState, ubn *UpdateBlockNum) {
	ubn.UpdatePeerBlockState(pbs)
}

func (c *Connection) ExpireBlkStateByBlockNum(num uint32) {
	kept := []*PeerBlockState{}
	for _, bs := range c.BlkState {
		if bs.BlockNum > num {
			kept = append(kept, bs)
		}
	}
	c.BlkState = kept
}
