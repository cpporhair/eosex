package network

import (
	"encoding/hex"
	"encoding/json"
	"eosex/network/config"
	"eosex/network/crypto"
	"fmt"
	"github.com/emicklei/go-restful"
	"github.com/pkg/errors"
	"io/ioutil"
	"strconv"
	"strings"
	"time"
)

type RestMgr struct {
}

func NewRestMgr() *RestMgr {
	return &RestMgr{}
}

func (r *RestMgr) Register(container *restful.Container) {

	chain_ws := new(restful.WebService)
	chain_ws.
		Path("/v1/chain").
		Consumes(restful.MIME_XML, restful.MIME_JSON, restful.MIME_OCTET).
		Produces(restful.MIME_XML, restful.MIME_JSON)

	chain_ws.Route(chain_ws.GET("/get_info").To(r.getInfo))
	chain_ws.Route(chain_ws.POST("/get_info").To(r.getInfo))
	chain_ws.Route(chain_ws.POST("/get_block").To(r.getBlock))
	chain_ws.Route(chain_ws.POST("/get_abi").To(r.getAbi))
	chain_ws.Route(chain_ws.POST("/get_code").To(r.getCode))
	chain_ws.Route(chain_ws.POST("/get_account").To(r.getAccount))
	chain_ws.Route(chain_ws.POST("/get_table_rows").To(r.getTableRows))
	chain_ws.Route(chain_ws.POST("/get_currency_balance").To(r.getCurrencyBalance))
	chain_ws.Route(chain_ws.POST("/get_currency_stats").To(r.getCurrencyStats))
	chain_ws.Route(chain_ws.POST("/get_producers").To(r.getProducers))
	chain_ws.Route(chain_ws.POST("/get_required_keys").To(r.getRequiredKeys))
	chain_ws.Route(chain_ws.POST("/push_transaction").To(r.pushTransaction))
	chain_ws.Route(chain_ws.POST("/abi_json_to_bin").To(r.pushAction))
	container.Add(chain_ws)

	wallet_ws := new(restful.WebService)
	wallet_ws.
		Path("/v1/wallet").
		Consumes(restful.MIME_XML, restful.MIME_JSON, restful.MIME_OCTET).
		Produces(restful.MIME_XML, restful.MIME_JSON)
	wallet_ws.Route(wallet_ws.POST("/create").To(r.walletCreate))
	wallet_ws.Route(wallet_ws.POST("/open").To(r.walletOpen))
	wallet_ws.Route(wallet_ws.POST("/lock").To(r.walletLock))
	wallet_ws.Route(wallet_ws.POST("/lock_all").To(r.walletLockAll))
	wallet_ws.Route(wallet_ws.POST("/unlock").To(r.walletUnlock))
	wallet_ws.Route(wallet_ws.POST("/import_key").To(r.walletImportKey))
	wallet_ws.Route(wallet_ws.POST("/remove_key").To(r.walletRemoveKey))
	wallet_ws.Route(wallet_ws.POST("/list_wallets").To(r.walletListWallets))
	wallet_ws.Route(wallet_ws.POST("/list_keys").To(r.walletListKeys))
	wallet_ws.Route(wallet_ws.POST("/get_public_keys").To(r.walletGetPublicKeys))
	container.Add(wallet_ws)

	history_ws := new(restful.WebService)
	history_ws.
		Path("/v1/history").
		Consumes(restful.MIME_XML, restful.MIME_JSON, restful.MIME_OCTET).
		Produces(restful.MIME_XML, restful.MIME_JSON)
	history_ws.Route(history_ws.POST("/get_actions").To(r.historyGetActions))
	history_ws.Route(history_ws.POST("/get_transaction").To(r.historyGetTransaction))
	history_ws.Route(history_ws.POST("/get_key_accounts").To(r.historyGetKeyAccounts))
	history_ws.Route(history_ws.POST("/get_controlled_accounts").To(r.historyGetControlledAccounts))
	container.Add(history_ws)

	net_ws := new(restful.WebService)
	net_ws.
		Path("/v1/net").
		Consumes(restful.MIME_XML, restful.MIME_JSON, restful.MIME_OCTET).
		Produces(restful.MIME_XML, restful.MIME_JSON)
	net_ws.Route(net_ws.POST("/connect").To(r.netConnect))
	net_ws.Route(net_ws.POST("/disconnect").To(r.netDisconnect))
	net_ws.Route(net_ws.POST("/status").To(r.netStatus))
	net_ws.Route(net_ws.POST("/connections").To(r.netConnections))
	container.Add(net_ws)
}

//get_info
func (r *RestMgr) getInfo(req *restful.Request, res *restful.Response) {
	result := GetChainInfoResult{}
	result.ServerVersion = "60947c0c"
	result.ChainId, _ = hex.DecodeString(config.ChainId)
	result.HeadBlockNum = GetChainMgr().GetChainHeadBlockNum()
	result.LastIrreversibleBlockNum = GetChainMgr().GetChainLibNum()
	result.HeadBlockId = GetChainMgr().GetChainHeadBlockId()
	result.LastIrreversibleBlockId = GetChainMgr().GetChainLibId()
	result.HeadBlockProducer = "eosio"
	result.HeadBlockTime = TimePoint{GetChainMgr().GetChainHeadBlockTime().Time}

	res.WriteAsJson(result)
}

type BlockBody struct {
	BlockNumOrId json.Number `json:"block_num_or_id"`
}

//get_block
func (r *RestMgr) getBlock(req *restful.Request, res *restful.Response) {

	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	body := &BlockBody{}
	err = json.Unmarshal(b, body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}

	blockNumOrId := body.BlockNumOrId.String()

	blkId := Sh256Hash([]byte(blockNumOrId))
	sb := GetChainMgr().GetBlockById(blkId)
	if sb == nil {
		blkNum, err := strconv.Atoi(blockNumOrId)
		if err != nil {
			fmt.Println(err)
		}
		sb = GetChainMgr().GetBlockByNumber(uint32(blkNum))
	}
	if sb == nil {
		errInfo := NewErrorInfo(3100002, "unknown_block_exception", "unknown block")
		errResult := NewErrorResults(500, "Internal Service Error", errInfo)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}

	result := GetBlockResult{}
	result.SignedBlock = *sb
	result.Id, err = sb.BlockID()
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	result.BlockNum = sb.BlockNumber()
	result.RefBlockPrefix = 1 //.......

	res.WriteAsJson(result)
}

type getAbiBody struct {
	AccountName AccountName `json:"account_name"`
}

///get_abi
func (r *RestMgr) getAbi(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	body := &getAbiBody{}
	err = json.Unmarshal(b, body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	accountName := body.AccountName
	fmt.Println(accountName)

	result := GetAbiResult{}
	result.AccountName = "eosio"
	result.Abi.Version = "eosio::abi/1.0"
	result.Abi.Types = []TypeDef{{"account_name", "name"}, {"permission_name", "name"}}
	result.Abi.Structs = []StructDef{}
	result.Abi.Actions = []ActionDef{}
	result.Abi.Tables = []TableDef{}
	result.Abi.RicardianClauses = []ClausePair{}
	result.Abi.ErrorMessages = []ErrorMessage{}
	result.Abi.AbiExtensions = ExtensionsType{}

	res.WriteAsJson(result)
}

type getCodeBody struct {
	AccountName AccountName `json:"account_name"`
	CodeAsWasm  bool        `json:"code_as_wasm"`
}

//get_code
func (r *RestMgr) getCode(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	body := &getCodeBody{}
	err = json.Unmarshal(b, body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	accountName := body.AccountName
	codeAsWasm := body.CodeAsWasm
	fmt.Println(accountName, codeAsWasm)

	result := GetCodeResult{}

	res.WriteAsJson(result)
}

type AccountBody struct {
	AccountName string `json:"account_name"`
}

//get_account
func (r *RestMgr) getAccount(req *restful.Request, res *restful.Response) {

	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	body := &AccountBody{}
	err = json.Unmarshal(b, body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}

	accName := body.AccountName
	fmt.Println(accName)

	result := GetAccountResult{}
	result.HeadBlockTime = TimePointSec{time.Now()}
	result.Created = TimePointSec{time.Now()}
	result.LastCodeUpdate = TimePointSec{time.Now()}
	result.Permissions = []Permission{}
	result.CoreLiquidBalance, _ = NewAsset("100.0000 SYS")

	res.WriteAsJson(result)
}

//get_table_rows
func (r *RestMgr) getTableRows(req *restful.Request, res *restful.Response) {
	result := GetTableRowsResult{}
	result.Rows = []byte{}
	res.WriteAsJson(result)
}

//get_currency_balance
func (r *RestMgr) getCurrencyBalance(req *restful.Request, res *restful.Response) {
	result := GetCurrencyBalanceResult{}

	ad, _ := NewAsset("100.0000 SYS")
	result.Balance = []Asset{ad}
	res.WriteAsJson(result.Balance)
}

//get_currency_stats
func (r *RestMgr) getCurrencyStats(req *restful.Request, res *restful.Response) {
	result := GetCurrencyStatsResult{}
	result.Issuer = "eosio"
	result.MaxSupply, _ = NewAsset("100000.0000 SYS")
	result.Supply, _ = NewAsset("10000.0000 SYS")
	res.WriteAsJson(result)
}

type getProducersBody struct {
	Json       bool   `json:"json"`
	LowerBound string `json:"lower_bound"`
	Limit      uint32 `json:"limit"`
}

//get_producers
func (r *RestMgr) getProducers(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	body := &getProducersBody{}
	err = json.Unmarshal(b, body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	fmt.Println(body)

	queryException := false
	if queryException {
		errInfo := NewErrorInfo(3060003, "contract_table_query_exception", "Contract Table Query Exception")
		errResult := NewErrorResults(500, "Internal Service Error", errInfo)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}

	pi := ProducerInfo{}
	pi.Owner = "lioninjungle"
	pi.TotalVotes = 1173461424815250688.00000000000000000
	pi.ProducerKey, _ = crypto.NewPublicKey("EOS5FBuqwwyu4xDSTRFs2RyZBUFfQASpwG4kthGQUWbMHtbkZ6hqW")
	pi.IsActive = true
	pi.Url = "http://cryptolions.io"
	pi.UnpaidBloks = 454035
	pi.LastClaimTime = uint64(time.Now().Unix())
	pi.Location = 0

	result := GetProducersResult{}
	result.Rows = []ProducerInfo{pi}
	res.WriteAsJson(result)
}

type KeyTransaction struct {
	//?????????
	Transaction
}

type getRequiredKeysBody struct {
	Transaction   KeyTransaction     `json:"transaction"`
	AvailableKeys []crypto.PublicKey `json:"available_keys"`
}

//get_required_keys
func (r *RestMgr) getRequiredKeys(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	name := string(b)
	fmt.Println(name)

	result := GetRequiredKeysResult{}
	result.RequiredKeys = []crypto.PublicKey{}

	res.WriteAsJson(result)
}

//push_transaction
func (r *RestMgr) pushTransaction(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	data := string(b)
	fmt.Println(data)

	result := PushTransactionResult{}

	res.WriteAsJson(result)
}

type PushActionArgs struct {
}

type pushActionBody struct {
	Code   AccountName    `json:"code"`
	Action ActionName     `json:"action"`
	Args   PushActionArgs `json:"args"`
}

//push action
func (r *RestMgr) pushAction(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	data := string(b)
	fmt.Println(data)

	body := &pushActionBody{}
	err = json.Unmarshal(b, body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	fmt.Println(body)

	result := PushActionResult{}
	res.WriteAsJson(result)
}

///////wallet

func (r *RestMgr) walletCreate(req *restful.Request, res *restful.Response) {

	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	name := string(b)
	fmt.Println(name)
	name = strings.Replace(name, "\"", "", -1)
	fmt.Println(name)

	res.Write([]byte("\"PW5JM7uzAsVhp6gJB6j6SnZTVf9QWfzbqwRBgzR5QoW6DcGjDabH1\""))
}

//wallet open
func (r *RestMgr) walletOpen(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	name := string(b)
	fmt.Println(name)
	name = strings.Replace(name, "\"", "", -1)
	fmt.Println(name)

	///////
	exist := true
	if !exist {
		errInfo := NewErrorInfo(3100002, "wallet_nonexistent_exception", "Nonexistent wallet")
		errResult := NewErrorResults(500, "Internal Service Error", errInfo)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	noErrorResult := NoErrorResults{}
	res.WriteAsJson(noErrorResult)
}

func (r *RestMgr) walletLock(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	name := string(b)
	fmt.Println(name)
	name = strings.Replace(name, "\"", "", -1)
	fmt.Println(name)

	exist := true
	if !exist {
		errInfo := NewErrorInfo(3100002, "wallet_nonexistent_exception", "Nonexistent wallet")
		errResult := NewErrorResults(500, "Internal Service Error", errInfo)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	noErrorResult := NoErrorResults{}
	res.WriteAsJson(noErrorResult)
}

func (r *RestMgr) walletLockAll(req *restful.Request, res *restful.Response) {

	noErrorResult := NoErrorResults{}
	res.WriteAsJson(noErrorResult)
}

func (r *RestMgr) walletUnlock(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	namePasswd := []string{}
	err = json.Unmarshal(b, &namePasswd)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	if len(namePasswd) != 2 {
		err = errors.New("param error")
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	name := namePasswd[0]
	psswd := namePasswd[1]
	fmt.Println(name, psswd)

	exist := true
	if !exist {
		errInfo := NewErrorInfo(3100002, "wallet_nonexistent_exception", "Nonexistent wallet")
		errResult := NewErrorResults(500, "Internal Service Error", errInfo)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	noErrorResult := NoErrorResults{}
	res.WriteAsJson(noErrorResult)
}

func (r *RestMgr) walletImportKey(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	namePrivKey := []string{}
	err = json.Unmarshal(b, &namePrivKey)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	if len(namePrivKey) != 2 {
		err = errors.New("param error")
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	name := namePrivKey[0]
	privKey := namePrivKey[1]
	fmt.Println(name, privKey)

	exist := true
	if !exist {
		errInfo := NewErrorInfo(3100002, "wallet_nonexistent_exception", "Nonexistent wallet")
		errResult := NewErrorResults(500, "Internal Service Error", errInfo)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	noErrorResult := NoErrorResults{}
	res.WriteAsJson(noErrorResult)
}

func (r *RestMgr) walletRemoveKey(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	reqParams := []string{}
	err = json.Unmarshal(b, &reqParams)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	if len(reqParams) != 3 {
		err = errors.New("param error")
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	name := reqParams[0]
	psswd := reqParams[1]
	key := reqParams[2]
	fmt.Println(name, psswd, key)

	exist := true
	if !exist {
		errInfo := NewErrorInfo(3100002, "wallet_nonexistent_exception", "Nonexistent wallet")
		errResult := NewErrorResults(500, "Internal Service Error", errInfo)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}

	locked := false
	if locked {
		errInfo := NewErrorInfo(3120003, "wallet_locked_exception", "Locked wallet")
		errResult := NewErrorResults(500, "Internal Service Error", errInfo)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	noErrorResult := NoErrorResults{}
	res.WriteAsJson(noErrorResult)
}

func (r *RestMgr) walletListWallets(req *restful.Request, res *restful.Response) {

	walletList := []string{"default", "zzz *"}
	res.WriteAsJson(walletList)
}

func (r *RestMgr) walletListKeys(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}

	namePasswd := []string{}
	err = json.Unmarshal(b, &namePasswd)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	if len(namePasswd) != 2 {
		err = errors.New("param error")
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	name := namePasswd[0]
	psswd := namePasswd[1]
	fmt.Println(name, psswd)

	exist := true
	if !exist {
		errInfo := NewErrorInfo(3100002, "wallet_nonexistent_exception", "Nonexistent wallet")
		errResult := NewErrorResults(500, "Internal Service Error", errInfo)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}

	keys := [2]string{"EOS7m229paPt64BRCg8EXv1SdeckaaGMK61pACZQKMQoAdFmqsqqx", "5JJd2gHrHe4NiUuCzdWjbZxPi7BGaFuNfisYAzitJeb2v3egBru"}
	keys2 := [2]string{"EOS7bVQjxGupHJk8Qimwz5rt8BCS6TxJGHa2TrAjUkXaGLk2mqq2M", "5JpPsaa8eekeLWW8uuqVfeRs97u4Z1LGEAnR6r41BcACRXc8tkJ"}
	walletKeys := [][2]string{keys, keys2}
	res.WriteAsJson(walletKeys)
}

func (r *RestMgr) walletGetPublicKeys(req *restful.Request, res *restful.Response) {
	walletKeys := []string{"EOS7m229paPt64BRCg8EXv1SdeckaaGMK61pACZQKMQoAdFmqsqqx"}
	res.WriteAsJson(walletKeys)
}

//history

type historyGetActionsBody struct {
	AccountName AccountName `json:"account_name"`
	Pos         int32       `json:"pos"`
	Offset      int32       `json:"offset"`
}

//get actions -j eosio 1 2
func (r *RestMgr) historyGetActions(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	body := &historyGetActionsBody{}
	err = json.Unmarshal(b, body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	fmt.Println(body)

	result := GetActionsResult{}
	result.Actions = []OrderedActionResult{}

	res.WriteAsJson(result)
}

type historyGetKeyAccountsBody struct {
	PublicKey crypto.PublicKey `json:"public_key"`
}

//get accounts
func (r *RestMgr) historyGetKeyAccounts(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	body := &historyGetKeyAccountsBody{}
	err = json.Unmarshal(b, body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	fmt.Println(body)

	result := GetKeyAccountsResults{}
	result.AccountNames = []AccountName{"eosio"}
	res.WriteAsJson(result)
}

type historyGetTransactionBody struct {
	Id string `json:"id"`
}

//cleos get transaction xxx
func (r *RestMgr) historyGetTransaction(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	body := &historyGetTransactionBody{}
	err = json.Unmarshal(b, body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	fmt.Println(body)
	tid, err := hex.DecodeString(body.Id)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}

	result := GetTransactionResult{}
	result.Id = tid
	result.BlockTime = BlockTimestamp{time.Now()}
	res.WriteAsJson(result)
}

type historyGetControlledAccountsBody struct {
	ControllingAccount AccountName `json:"controlling_account"`
}

//cleos get servants eosio
func (r *RestMgr) historyGetControlledAccounts(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	body := &historyGetControlledAccountsBody{}
	err = json.Unmarshal(b, body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	fmt.Println(body)

	result := GetControlledAccountsResults{}
	result.ControlledAccounts = []AccountName{"eosio"}
	res.WriteAsJson(result)
}

//net
//net connect
func (r *RestMgr) netConnect(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	host := string(b)
	host = strings.Replace(host, "\"", "", -1)
	fmt.Println(host)

	added := "\"added connection\""
	res.Write([]byte(added))
}

//net disconnect
func (r *RestMgr) netDisconnect(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	host := string(b)
	host = strings.Replace(host, "\"", "", -1)
	fmt.Println(host)

	removed := "\"connection removed\""
	res.Write([]byte(removed))
}

//net status
func (r *RestMgr) netStatus(req *restful.Request, res *restful.Response) {
	b, err := ioutil.ReadAll(req.Request.Body)
	if err != nil {
		fmt.Println(err)
		errResult := NewErrorResultsByError(err)
		res.WriteHeaderAndJson(500, errResult, restful.MIME_JSON)
		return
	}
	host := string(b)
	host = strings.Replace(host, "\"", "", -1)
	fmt.Println(host)

	result := NetStatusResult{}
	result.Peer = host
	result.Connecting = true
	result.Syncing = false
	result.LastHandshake = HandshakeMessage{}
	res.WriteAsJson(result)
}

//net peers
func (r *RestMgr) netConnections(req *restful.Request, res *restful.Response) {

	result := NetStatusResult{}
	result.Peer = "127.0.0.1:9876"
	result.Connecting = true
	result.Syncing = false
	result.LastHandshake = HandshakeMessage{}

	results := []NetStatusResult{result}
	res.WriteAsJson(results)
}
