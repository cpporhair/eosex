package network

import (
	"sync"
)

var chainOnce sync.Once
var chainMgr *ChainMgr

type ChainMgr struct {
}

func GetChainMgr() *ChainMgr {
	chainOnce.Do(func() {
		chainMgr = &ChainMgr{}
	})
	return chainMgr
}

func (m *ChainMgr) GetChainLibNum() uint32 {
	//test...
	libNum := GetLocalDataInstance().GetLib()
	return libNum
}

func (m *ChainMgr) GetChainLibId() Sha256Type {
	id := GetLocalDataInstance().GetLibID()
	return id
}

func (m *ChainMgr) GetChainHeadBlockNum() uint32 {
	num := GetLocalDataInstance().GetHeadBlockNum()
	return num
}

func (m *ChainMgr) GetChainHeadBlockId() Sha256Type {
	id := GetLocalDataInstance().GetHeadBlockID()
	return id
}

func (m *ChainMgr) GetChainHeadBlockTime() BlockTimestamp {
	blockTime := GetLocalDataInstance().GetHeadBlockTime()
	return blockTime
}

func (m *ChainMgr) GetChainBlockIdForNum(num uint32) Sha256Type {
	return NewEmptySha256Type()
}

func (m *ChainMgr) GetBlockByNumber(num uint32) *SignedBlock {
	return nil
}

func (m *ChainMgr) GetBlockById(blkId Sha256Type) *SignedBlock {
	return nil
}

func (m *ChainMgr) AcceptBlock(block *SignedBlock) int32 {
	return 0
}

func (m *ChainMgr) AcceptTransaction(trx *PackedTransaction) int32 {
	return 0
}
