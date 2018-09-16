package network

import (
	"encoding/json"
	"eosex/network/config"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"sync"
	"time"
)

var once sync.Once

var localDataInstance *LocalData

//test temp data
type LocalData struct {
	HeadBlockNum             uint32
	LastIrreversibleBlockNum uint32
	LastIrreversibleBlockID  Sha256Type
	HeadBlockID              Sha256Type
	HeadBlockTime            BlockTimestamp
	SyncEndBlockNum          uint32
	PeerHeadBlockNum         uint32
	Syncing                  bool
	lock                     sync.RWMutex
}

func GetLocalDataInstance() *LocalData {
	once.Do(func() {
		info := getChainInfo()
		//time.Sleep(time.Second * 3)
		if info != nil {
			GetConnsMgr().syncMgr.syncLastRequestedNum = info.LastIrreversibleBlockNum
			GetConnsMgr().syncMgr.syncNextExpectedNum = info.LastIrreversibleBlockNum + 1
			localDataInstance = &LocalData{
				HeadBlockNum:             info.LastIrreversibleBlockNum,
				LastIrreversibleBlockNum: info.LastIrreversibleBlockNum,
				LastIrreversibleBlockID:  info.LastIrreversibleBlockId,
				HeadBlockID:              info.LastIrreversibleBlockId,
				HeadBlockTime:            BlockTimestamp{info.HeadBlockTime.Time},
			}
		} else {
			localDataInstance = &LocalData{
				HeadBlockNum:             0,
				LastIrreversibleBlockNum: 0,
				LastIrreversibleBlockID:  NewEmptySha256Type(),
				HeadBlockID:              NewEmptySha256Type(),
				HeadBlockTime:            BlockTimestamp{time.Now()},
			}
		}
	})
	return localDataInstance
}

func (l *LocalData) GetHeadBlockNum() uint32 {
	l.lock.RLock()
	defer l.lock.RUnlock()

	return l.HeadBlockNum
}

func (l *LocalData) SetHeadBlockNum(num uint32) {
	l.lock.RLock()
	defer l.lock.RUnlock()

	l.HeadBlockNum = num
}

func (l *LocalData) GetLib() uint32 {
	l.lock.RLock()
	defer l.lock.RUnlock()

	return l.LastIrreversibleBlockNum
}

func (l *LocalData) SetLib(lib uint32) {
	l.lock.RLock()
	defer l.lock.RUnlock()

	l.LastIrreversibleBlockNum = lib
}

func (l *LocalData) GetLibID() Sha256Type {
	l.lock.RLock()
	defer l.lock.RUnlock()

	return l.LastIrreversibleBlockID
}

func (l *LocalData) SetLibID(libId Sha256Type) {
	l.lock.RLock()
	defer l.lock.RUnlock()

	l.LastIrreversibleBlockID = libId
}

func (l *LocalData) GetHeadBlockID() Sha256Type {
	l.lock.RLock()
	defer l.lock.RUnlock()

	return l.HeadBlockID
}

func (l *LocalData) SetHeadBlockID(headId Sha256Type) {
	l.lock.RLock()
	defer l.lock.RUnlock()

	l.HeadBlockID = headId
}

func (l *LocalData) GetHeadBlockTime() BlockTimestamp {
	l.lock.RLock()
	defer l.lock.RUnlock()

	return l.HeadBlockTime
}

func (l *LocalData) SetHeadBlockTime(bt BlockTimestamp) {
	l.lock.RLock()
	defer l.lock.RUnlock()

	l.HeadBlockTime = bt
}

func (l *LocalData) SetHeadData(headBlockNum uint32, headBlockID Sha256Type, headBlockTime BlockTimestamp) {
	l.lock.Lock()
	defer l.lock.Unlock()

	l.HeadBlockNum = headBlockNum
	l.HeadBlockID = headBlockID
	l.HeadBlockTime = headBlockTime
}

func (l *LocalData) SetLibData(libNum uint32, libID Sha256Type) {
	l.lock.Lock()
	defer l.lock.Unlock()

	l.LastIrreversibleBlockNum = libNum
	l.LastIrreversibleBlockID = libID
}

func (l *LocalData) SetLocalData(headBlockNum, libNum uint32, headBlockID, libID Sha256Type, headBlockTime BlockTimestamp) {
	l.lock.Lock()
	defer l.lock.Unlock()

	l.HeadBlockNum = headBlockNum
	l.HeadBlockID = headBlockID
	l.LastIrreversibleBlockNum = libNum
	l.LastIrreversibleBlockID = libID
	l.HeadBlockTime = headBlockTime
}

func getChainInfo() *GetChainInfoResult {
	url := fmt.Sprintf("%s/v1/chain/get_info", config.HttpServerAddr)
	res, err := http.Post(url, "application/json;charset=utf-8", nil)
	if err != nil {
		log.Fatal(err)
		return nil
	}
	body, err := ioutil.ReadAll(res.Body)
	res.Body.Close()
	if err != nil {
		log.Fatal(err)
		return nil
	}
	fmt.Printf("%s", body)

	info := NewGetChainInfoResult()
	//err = UnmarshalBinary(body, &info)
	err = json.Unmarshal(body, info)
	if err != nil {
		fmt.Println(err)
		return nil
	}
	fmt.Println(info)
	return info
}
