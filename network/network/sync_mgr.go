package network

import (
	"bytes"
	"eosex/network/config"
)

type StagesType int

const (
	LibCatchup StagesType = iota
	HeadCatchup
	InSync
)

type SyncMgr struct {
	syncKnownLibNum      uint32
	syncLastRequestedNum uint32
	syncNextExpectedNum  uint32
	syncReqSpan          uint32
	source               *Connection
	state                StagesType
	blocks               []BlockIdType
}

func NewSyncMgr(span uint32) *SyncMgr {
	return &SyncMgr{
		syncKnownLibNum:      0,
		syncLastRequestedNum: 0,
		syncNextExpectedNum:  1,
		syncReqSpan:          span,
		state:                InSync,
	}
}

func (m *SyncMgr) StageStr(s StagesType) string {
	switch s {
	case InSync:
		return "in sync"
	case LibCatchup:
		return "lib catchup"
	case HeadCatchup:
		return "head catchup"
	default:
		return "unkown"
	}
}

func (m *SyncMgr) SetState(newstate StagesType) {
	if m.state == newstate {
		return
	}
	m.state = newstate
}

func (m *SyncMgr) IsActive(c *Connection) bool {
	if m.state == HeadCatchup && c != nil {
		fhset := !c.ForkHead.IsEmpty()
		return fhset && c.ForkHeadNum < GetChainMgr().GetChainHeadBlockNum()
	}
	return m.state != InSync
}

func (m *SyncMgr) ResetLibNum(c *Connection) {
	if m.state == InSync {
		m.source = nil
	}
	if c.Current() {
		if c.LastHandshakeRecv.LastIrreversibleBlockNum > m.syncKnownLibNum {
			m.syncKnownLibNum = c.LastHandshakeRecv.LastIrreversibleBlockNum
		}
	} else if c == m.source {
		m.syncLastRequestedNum = 0
		m.requestNextChunk(nil)
	}
}

func (m *SyncMgr) syncRequired() bool {
	return (m.syncLastRequestedNum < m.syncKnownLibNum || GetChainMgr().GetChainHeadBlockNum() < m.syncLastRequestedNum)
}

func (m *SyncMgr) requestNextChunk(c *Connection) {
	head_block := GetChainMgr().GetChainHeadBlockNum()
	if head_block < m.syncLastRequestedNum && m.source != nil && m.source.Current() {
		//fc_ilog (logger, "ignoring request, head is ${h} last req = ${r} source is ${p}", ("h",head_block)("r",sync_last_requested_num)("p",source->peer_name()));
		return
	}
	if c != nil && c.Current() {
		m.source = c
	} else {
		if len(GetConnsMgr().Connections) == 1 {
			if m.source == nil {
				m.source = GetConnsMgr().Connections[0]
			}
		} else {
			if m.source != nil {
				index := GetConnsMgr().findConnIndex(m.source)
				if index == -1 {
					m.source = nil
				}
			}
			for _, c := range GetConnsMgr().Connections {
				if c != m.source && c.Current() {
					m.source = c
					break
				}
			}
		}
	}
	if m.source == nil || !m.source.Current() {
		m.syncKnownLibNum = GetChainMgr().GetChainLibNum()
		m.syncLastRequestedNum = 0
		m.SetState(InSync)
		return
	}
	if m.syncLastRequestedNum != m.syncKnownLibNum {
		start := m.syncNextExpectedNum
		end := start + m.syncReqSpan - 1
		if end > m.syncKnownLibNum {
			end = m.syncKnownLibNum
		}
		if end > 0 && end >= start {
			m.source.RequestSyncBlocks(start, end)
			m.syncLastRequestedNum = end
		}
	}
}

func (m *SyncMgr) sendHandshakes() {
	for _, c := range GetConnsMgr().Connections {
		if c.Current() {
			c.SendHandshake()
		}
	}
}

func (m *SyncMgr) startSync(c *Connection, target uint32) {
	if target > m.syncKnownLibNum {
		m.syncKnownLibNum = target
	}
	if !m.syncRequired() {
		return
	}
	if m.state == InSync {
		m.SetState(LibCatchup)
		m.syncNextExpectedNum = GetChainMgr().GetChainLibNum() + 1
	}
	m.requestNextChunk(c)
}

func (m *SyncMgr) recvHandshake(c *Connection, msg *HandshakeMessage) {
	libNum := GetChainMgr().GetChainLibNum()
	peerLibNum := msg.LastIrreversibleBlockNum
	m.resetLibNum(c)
	c.Syncing = false
	//--------------------------------
	// sync need checks; (lib == last irreversible block)
	//
	// 0. my head block id == peer head id means we are all caugnt up block wise
	// 1. my head block num < peer lib - start sync locally
	// 2. my lib > peer head num - send an last_irr_catch_up notice if not the first generation
	//
	// 3  my head block num <= peer head block num - update sync state and send a catchup request
	// 4  my head block num > peer block num ssend a notice catchup if this is not the first generation
	//
	//-----------------------------
	headNum := GetChainMgr().GetChainHeadBlockNum()
	headId := GetChainMgr().GetChainHeadBlockId()
	if bytes.Equal(headId, msg.HeadID) {
		note := &NoticeMessage{}
		note.KnownBlocks.Mode = None
		note.KnownTrx.Mode = CatchUp
		note.KnownTrx.Pending = uint32(len(GetConnsMgr().localTxns))
		c.Enqueue(note)
		return
	}
	if headNum < peerLibNum {
		if c.ProtocolVersion < config.ProtoExplicitSync {
			m.startSync(c, peerLibNum)
		}
		return
	}
	if libNum > msg.HeadNum {
		if msg.Generation > 1 || c.ProtocolVersion > config.ProtoBase {
			note := &NoticeMessage{}
			note.KnownTrx.Pending = libNum
			note.KnownTrx.Mode = LastIrrCatchUp
			note.KnownBlocks.Mode = LastIrrCatchUp
			note.KnownBlocks.Pending = headNum
			c.Enqueue(note)
		}
		c.Syncing = true
		return
	}
	if headNum < msg.HeadNum {
		m.verifyCatchup(c, msg.HeadNum, msg.HeadID)
		return
	} else {
		if msg.Generation > 1 || c.ProtocolVersion > config.ProtoBase {
			note := &NoticeMessage{}
			note.KnownTrx.Mode = None
			note.KnownBlocks.Mode = CatchUp
			note.KnownBlocks.Pending = headNum
			note.KnownBlocks.IDs = append(note.KnownBlocks.IDs, headId)
			c.Enqueue(note)
		}
		c.Syncing = true
		return
	}
}

func (m *SyncMgr) resetLibNum(c *Connection) {
	if m.state == InSync {
		m.source = nil
	}
	if c.Current() {
		if c.LastHandshakeRecv.LastIrreversibleBlockNum > m.syncKnownLibNum {
			m.syncKnownLibNum = c.LastHandshakeRecv.LastIrreversibleBlockNum
		}
	} else if c == m.source {
		m.syncLastRequestedNum = 0
		m.requestNextChunk(nil)
	}
}

func (m *SyncMgr) verifyCatchup(c *Connection, num uint32, id Sha256Type) {
	req := &RequestMessage{}
	req.ReqTrx.IDs = []Sha256Type{}
	req.ReqBlocks.Mode = CatchUp
	for _, c := range GetConnsMgr().Connections {
		if bytes.Equal(c.ForkHead, id) || c.ForkHeadNum > num {
			req.ReqBlocks.Mode = None
			break
		}
	}
	if req.ReqBlocks.Mode == CatchUp {
		c.ForkHead = id
		c.ForkHeadNum = num
		if m.state == LibCatchup {
			return
		}
		req.ReqBlocks.IDs = []Sha256Type{}
		m.SetState(HeadCatchup)
	} else {
		c.ForkHead = NewEmptySha256Type()
		c.ForkHeadNum = 0
	}
	req.ReqTrx.Mode = None
	c.Enqueue(req)
}

func (m *SyncMgr) recvNotice(c *Connection, msg *NoticeMessage) {
	if msg.KnownBlocks.Mode == CatchUp {
		if len(msg.KnownBlocks.IDs) == 0 {
			//elog ("got a catch up with ids size = 0");
		} else {
			m.verifyCatchup(c, msg.KnownBlocks.Pending, msg.KnownBlocks.IDs[len(msg.KnownBlocks.IDs)-1])
		}
	} else {
		c.LastHandshakeRecv.LastIrreversibleBlockNum = msg.KnownTrx.Pending
		m.ResetLibNum(c)
		m.startSync(c, msg.KnownBlocks.Pending)
	}
}

func (m *SyncMgr) rejectedBlock(c *Connection, blkNum uint32) {
	if m.state != InSync {
		m.syncLastRequestedNum = 0
		m.source = nil
		GetConnsMgr().Close(c)
		m.SetState(InSync)
		m.sendHandshakes()
	}
}

func (m *SyncMgr) recvBlock(c *Connection, blkId Sha256Type, blkNum uint32) {

	if m.state == LibCatchup {
		if blkNum != m.syncNextExpectedNum {
			GetConnsMgr().Close(c)
			return
		}
		m.syncNextExpectedNum = blkNum + 1
		localData := GetLocalDataInstance()
		localData.SetLibData(blkNum, blkId)
	}
	if m.state == HeadCatchup {
		m.SetState(InSync)
		m.source = nil
		nullId := []byte{}
		for _, c := range GetConnsMgr().Connections {
			if bytes.Equal(c.ForkHead, nullId) {
				continue
			}
			if bytes.Equal(c.ForkHead, blkId) || c.ForkHeadNum < blkNum {
				c.ForkHead = nullId
				c.ForkHeadNum = 0
			} else {
				m.SetState(HeadCatchup)
			}
		}
	} else if m.state == LibCatchup {
		if blkNum == m.syncKnownLibNum {
			m.SetState(InSync)
			m.sendHandshakes()
		} else if blkNum == m.syncLastRequestedNum {
			m.requestNextChunk(nil)
		} else {
			//fc_dlog(logger,"calling sync_wait on connection ${p}",("p",c->peer_name()));
			//c->sync_wait();
		}
	}

}
