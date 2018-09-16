package network

import (
	"encoding/json"
	"eosex/network/crypto"
)

type ChainIdType Sha256Type

type GetChainInfoResult struct {
	ServerVersion            string      `json:"server_version"`
	ChainId                  Sha256Type  `json:"chain_id"`
	HeadBlockNum             uint32      `json:"head_block_num"`
	LastIrreversibleBlockNum uint32      `json:"last_irreversible_block_num"`
	LastIrreversibleBlockId  Sha256Type  `json:"last_irreversible_block_id"`
	HeadBlockId              Sha256Type  `json:"head_block_id"`
	HeadBlockTime            TimePoint   `json:"head_block_time"`
	HeadBlockProducer        AccountName `json:"head_block_producer"`
	VirtualBlockCpuLimit     uint64      `json:"virtual_block_cpu_limit"`
	VirtualBlockNetLimit     uint64      `json:"virtual_block_net_limit"`
	BlockCpuLimit            uint64      `json:"block_cpu_limit"`
	BlockNetLimit            uint64      `json:"block_net_limit"`
}

func NewGetChainInfoResult() *GetChainInfoResult {
	return &GetChainInfoResult{
		ChainId:                 NewEmptySha256Type(),
		LastIrreversibleBlockId: NewEmptySha256Type(),
		HeadBlockId:             NewEmptySha256Type(),
	}
}

type GetBlockResult struct {
	SignedBlock
	Id             Sha256Type `json:"id"`
	BlockNum       uint32     `json:"block_num"`
	RefBlockPrefix uint32     `json:"ref_block_prefix"`
}

//func (m *GetBlockResult) MarshalJSON() []byte {
//	data, err := MarshalBinary(m)
//	if err != nil {
//		return nil
//	}
//	return data
//}
//
//
//func (m *GetBlockResult) UnmarshalJSON(data []byte) error {
//	err := json.Unmarshal(data, m)
//	if err != nil {
//		return err
//	}
//	return nil
//}

type GetAbiResult struct {
	AccountName AccountName `json:"account_name"`
	Abi         ABIDef      `json:"abi"`
}

type GetCodeResult struct {
	AccountName AccountName `json:"account_name"`
	Wast        string      `json:"wast"`
	Wasm        string      `json:"wasm"`
	CodeHash    Sha256Type  `json:"code_hash"`
	Abi         *ABIDef     `json:"abi"`
}

type GetAccountResult struct {
	AccountName            AccountName          `json:"account_name"`
	HeadBlockNum           uint32               `json:"head_block_num"`
	HeadBlockTime          TimePointSec         `json:"head_block_time"`
	Privileged             bool                 `json:"privileged"`
	LastCodeUpdate         TimePointSec         `json:"last_code_update"`
	Created                TimePointSec         `json:"created"`
	CoreLiquidBalance      Asset                `json:"core_liquid_balance"`
	RAMQuota               int64                `json:"ram_quota"`
	NetWeight              int64                `json:"net_weight"`
	CPUWeight              int64                `json:"cpu_weight"`
	NetLimit               AccountResourceLimit `json:"net_limit"`
	CPULimit               AccountResourceLimit `json:"cpu_limit"`
	RAMUsage               int64                `json:"ram_usage"`
	Permissions            []Permission         `json:"permissions"`
	TotalResources         *TotalResources      `json:"total_resources"`
	SelfDelegatedBandwidth *DelegatedBandwidth  `json:"self_delegated_bandwidth"`
	RefundRequest          *RefundRequest       `json:"refund_request"`
	VoterInfo              *VoterInfo           `json:"voter_info"`
}

func (r *GetAccountResult) MarshalJSON() []byte {
	data, err := MarshalBinary(r)
	if err != nil {
		return nil
	}
	return data
}

func (r *GetAccountResult) UnmarshalJSON(data []byte) error {
	err := json.Unmarshal(data, r)
	if err != nil {
		return err
	}
	return nil
}

type GetTableRowsResult struct {
	Rows []byte `json:"rows"`
	More bool   `json:"more"`
}

type GetCurrencyBalanceResult struct {
	Balance []Asset
}

type GetCurrencyStatsResult struct {
	Supply    Asset       `json:"supply"`
	MaxSupply Asset       `json:"max_supply"`
	Issuer    AccountName `json:"issuer"`
}

type ProducerInfo struct {
	Owner         AccountName      `json:"owner"`
	TotalVotes    float64          `json:"total_votes"`
	ProducerKey   crypto.PublicKey `json:"producer_key"`
	IsActive      bool             `json:"is_active"`
	Url           string           `json:"url"`
	UnpaidBloks   uint32           `json:"unpaid_bloks"`
	LastClaimTime uint64           `json:"last_claim_time"`
	Location      uint16           `json:"location"`
}

type GetProducersResult struct {
	Rows                    []ProducerInfo `json:"rows"`
	TotalProducerVoteWeight float64        `json:"total_producer_vote_weight"`
	More                    string         `json:"more"`
}

type GetRequiredKeysResult struct {
	RequiredKeys []crypto.PublicKey `json:"required_keys"`
}

type PushTransactionResult struct {
	TransactionID ChecksumType `json:"transaction_id"`
	//Processed     variant         `json:"processed"` //fc::variant
}

type PushActionResult struct {
}

type GetTransactionResult struct {
	Id                    Sha256Type        `json:"id"`
	Trx                   SignedTransaction `json:"trx"`
	BlockTime             BlockTimestamp    `json:"block_time"`
	BlockNum              uint32            `json:"block_num"`
	LastIrreversibleBlock uint32            `json:"last_irreversible_block"`
	Traces                []ActionTrace     `json:"traces"`
}

type GetControlledAccountsResults struct {
	ControlledAccounts []AccountName `json:"controlled_accounts"`
}

type OrderedActionResult struct {
	GlobalActionSeq  uint64         `json:"global_action_seq"`
	AccountActionSeq int32          `json:"account_action_seq"`
	BlockNum         uint32         `json:"block_num"`
	BlockTime        BlockTimestamp `json:"block_time"`
	ActionTrace      ActionTrace    `json:"action_trace"`
}

type GetActionsResult struct {
	Actions                []OrderedActionResult `json:"actions"`
	LastIrreversibleBlock  uint32                `json:"last_irreversible_block"`
	TimeLimitExceededError bool                  `json:"time_limit_exceeded_error"`
}

type GetKeyAccountsResults struct {
	AccountNames []AccountName `json:"account_names"`
}

type NetStatusResult struct {
	Peer          string           `json:"peer"`
	Connecting    bool             `json:"connecting"`
	Syncing       bool             `json:"syncing"`
	LastHandshake HandshakeMessage `json:"last_handshake"`
}
