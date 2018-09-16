package network

import (
	"bytes"
	"eosex/network/config"
	"time"
	"unsafe"
)

type BlockRequest struct {
	Id         Sha256Type
	LocalRetry bool
}

type BlockOrigin struct {
	Id     Sha256Type
	Origin *Connection
}

type TransactionOrigin struct {
	Id     Sha256Type
	Origin *Connection
}

type DispatchMgr struct {
	justSendItMax        uint32
	reqBlks              []BlockRequest
	reqTrx               []Sha256Type
	receivedBlocks       []BlockOrigin
	receivedTransactions []TransactionOrigin
}

func NewDispatchMgr() *DispatchMgr {
	return &DispatchMgr{
		justSendItMax: config.MaxImplicitRequest,
	}
}

func (m *DispatchMgr) bcastBlock(blk *SignedBlock) {
	blkId, err := blk.BlockID()
	if err != nil {
		return
	}
	var skip *Connection
	for i := 0; i < len(m.receivedBlocks); i++ {
		if bytes.Equal(m.receivedBlocks[i].Id, blkId) {
			skip = m.receivedBlocks[i].Origin
			m.receivedBlocks = append(m.receivedBlocks[:i], m.receivedBlocks[i+1:]...)
			break
		}
	}

	packsize := uint32(len(blk.Serialize())) + 1 //??
	msgsize := packsize + uint32(unsafe.Sizeof(packsize))

	pendingNotify := &NoticeMessage{}
	pendingNotify.KnownBlocks.Mode = Normal
	pendingNotify.KnownBlocks.IDs = append(pendingNotify.KnownBlocks.IDs, blkId)
	pendingNotify.KnownTrx.Mode = None

	tp := TimePoint{}
	pbs := &PeerBlockState{blkId, blk.BlockNumber(), false, true, tp}
	if (config.LargeMsgNotify && msgsize > m.justSendItMax) && skip != nil {
		GetConnsMgr().SendAll(pendingNotify, func(c *Connection) bool {
			if skip == c || !c.Current() {
				return false
			}
			unknown := c.AddPeerBlock(pbs)
			if !unknown {
				//elog("${p} already has knowledge of block ${b}", ("p",c->peer_name())("b",pbstate.block_num));
			}
			return unknown
		})
	} else {
		pbs.IsKnown = true
		for _, c := range GetConnsMgr().Connections {
			if c == skip || !c.Current() {
				continue
			}
			c.AddPeerBlock(pbs)
			c.Enqueue(blk)
		}
	}
}

func (m *DispatchMgr) recvBlock(c *Connection, id Sha256Type, bnum uint32) {
	origin := BlockOrigin{id, c}
	m.receivedBlocks = append(m.receivedBlocks, origin)
	if c != nil && c.LastReq != nil && c.LastReq.ReqBlocks.Mode != None && bytes.Equal(c.LastReq.ReqBlocks.IDs[len(c.LastReq.ReqBlocks.IDs)-1], id) {
		c.LastReq = nil
	}
	tp := TimePoint{}
	pbs := &PeerBlockState{id, bnum, false, true, tp}
	c.AddPeerBlock(pbs)
}

func (m *DispatchMgr) rejectedBlock(id Sha256Type) {
	for i := 0; i < len(m.receivedBlocks); i++ {
		if bytes.Equal(m.receivedBlocks[i].Id, id) {
			m.receivedBlocks = append(m.receivedBlocks[:i], m.receivedBlocks[i+1:]...)
			break
		}
	}
}

func (m *DispatchMgr) bcastTransaction(trx *PackedTransaction) {
	id := trx.ID()
	var skip *Connection
	for i := 0; i < len(m.receivedTransactions); i++ {
		if bytes.Equal(m.receivedTransactions[i].Id, id) {
			skip = m.receivedTransactions[i].Origin
			m.receivedTransactions = append(m.receivedTransactions[:i], m.receivedTransactions[i+1:]...)
			break
		}
	}
	for i := 0; i < len(m.reqTrx); i++ {
		if bytes.Equal(m.reqTrx[i], id) {
			m.reqTrx = append(m.reqTrx[:i], m.reqTrx[i+1:]...)
			break
		}
	}
	if GetConnsMgr().FindLocalTxnsById(id) != nil {
		return
	}
	packsize := uint32(len(trx.Serialize())) + 1 //??
	msgsize := packsize + uint32(unsafe.Sizeof(packsize))
	trxExpire := trx.unpackedTrx.Expiration
	buff := []byte{}

	nts := &NodeTransactionState{id, trxExpire, trx, buff, 0, 0, 0}
	GetConnsMgr().localTxns = append(GetConnsMgr().localTxns, nts)
	if !config.LargeMsgNotify || msgsize <= m.justSendItMax {
		GetConnsMgr().SendAll(trx, func(c *Connection) bool {
			if c == skip || c.Syncing {
				return false
			}
			bs := c.FindTrxStateById(id)
			unknown := bs == nil
			if unknown {
				ts := &TransactionState{id, true, true, 0, trxExpire, TimePoint{}}
				c.TrxState = append(c.TrxState, ts)
			} else {
				ute := &UpdateTxnExpiry{trxExpire}
				c.ModifyTrxStateByTxnExpiry(bs, ute)
			}
			return unknown
		})
	} else {
		pendingNotify := &NoticeMessage{}
		pendingNotify.KnownTrx.Mode = Normal
		pendingNotify.KnownTrx.IDs = append(pendingNotify.KnownTrx.IDs, id)
		pendingNotify.KnownBlocks.Mode = None
		GetConnsMgr().SendAll(pendingNotify, func(c *Connection) bool {
			if c == skip || c.Syncing {
				return false
			}
			bs := c.FindTrxStateById(id)
			unknown := bs == nil
			if unknown {
				ts := &TransactionState{id, false, true, 0, trxExpire, TimePoint{}}
				c.TrxState = append(c.TrxState, ts)
			} else {
				ute := &UpdateTxnExpiry{trxExpire}
				c.ModifyTrxStateByTxnExpiry(bs, ute)
			}
			return unknown
		})
	}
}

func (m *DispatchMgr) recvTransaction(c *Connection, id Sha256Type) {
	origin := TransactionOrigin{id, c}
	m.receivedTransactions = append(m.receivedTransactions, origin)
	if c != nil && c.LastReq != nil && c.LastReq.ReqTrx.Mode != None && bytes.Equal(c.LastReq.ReqTrx.IDs[len(c.LastReq.ReqTrx.IDs)-1], id) {
		c.LastReq = nil
	}
}

func (m *DispatchMgr) rejectedTransaction(id Sha256Type) {
	for i := 0; i < len(m.receivedTransactions); i++ {
		if bytes.Equal(m.receivedTransactions[i].Id, id) {
			m.receivedTransactions = append(m.receivedTransactions[:i], m.receivedTransactions[i+1:]...)
			break
		}
	}
}

func (m *DispatchMgr) RecvNotice(c *Connection, msg *NoticeMessage, generated bool) {
	req := &RequestMessage{}
	req.ReqTrx.Mode = None
	req.ReqBlocks.Mode = None
	sendReq := false
	if msg.KnownTrx.Mode == Normal {
		req.ReqTrx.Mode = Normal
		req.ReqTrx.Pending = 0
		for i := 0; i < len(msg.KnownTrx.IDs); i++ {
			tx := GetConnsMgr().FindLocalTxnsById(msg.KnownTrx.IDs[i])
			expires := TimePointSec{time.Now().Add(120 * time.Second)}
			ts := &TransactionState{msg.KnownTrx.IDs[i], true, true, 0, expires, TimePoint{}}
			if tx == nil {
				c.TrxState = append(c.TrxState, ts)
				req.ReqTrx.IDs = append(req.ReqTrx.IDs, msg.KnownTrx.IDs[i])
				m.reqTrx = append(m.reqTrx, msg.KnownTrx.IDs[i])
			}
		}
		sendReq = len(req.ReqTrx.IDs) > 0
	} else if msg.KnownTrx.Mode != None {
		return
	}
	if msg.KnownBlocks.Mode == Normal {
		req.ReqBlocks.Mode = Normal
		for i := 0; i < len(msg.KnownBlocks.IDs); i++ {
			var b *SignedBlock
			entry := &PeerBlockState{msg.KnownBlocks.IDs[i], 0, true, true, TimePoint{}}
			//b = cc.fetch_block_by_id(blkid);
			if b != nil {
				entry.BlockNum = b.BlockNumber()
			}
			if b == nil {
				sendReq = true
				req.ReqBlocks.IDs = append(req.ReqBlocks.IDs, msg.KnownBlocks.IDs[i])
				bq := BlockRequest{msg.KnownBlocks.IDs[i], generated}
				m.reqBlks = append(m.reqBlks, bq)
				entry.RequestedTime = TimePoint{time.Now()}
			}
			c.AddPeerBlock(entry)
		}
	} else if msg.KnownBlocks.Mode != None {
		return
	}
	if sendReq {
		c.Enqueue(req)
		c.LastReq = req
	}
}

func (m *DispatchMgr) retryFetch(c *Connection) {
	if c.LastReq == nil {
		return
	}
	var tid Sha256Type
	var bid Sha256Type
	isTxn := false
	if c.LastReq.ReqTrx.Mode == Normal {
		isTxn = true
		tid = c.LastReq.ReqTrx.IDs[len(c.LastReq.ReqTrx.IDs)-1]
	} else if c.LastReq.ReqBlocks.Mode == Normal {
		bid = c.LastReq.ReqBlocks.IDs[len(c.LastReq.ReqBlocks.IDs)-1]
	} else {
		return
	}
	for _, conn := range GetConnsMgr().Connections {
		if conn == c || conn.LastReq != nil {
			continue
		}
		sendit := false
		if isTxn {
			trx := conn.FindTrxStateById(tid)
			sendit = trx != nil && trx.IsKnownByPeer
		} else {
			blk := conn.FindPeerBlockStateById(bid)
			sendit = blk != nil && blk.IsKnown
		}
		if sendit {
			conn.Enqueue(c.LastReq)
			//conn->fetch_wait();
			conn.LastReq = c.LastReq
			return
		}
	}
	if c.Connected() {
		c.Enqueue(c.LastReq)
	}
}
