package network

type Action struct {
	Account       AccountName       `json:"account"`
	Name          ActionName        `json:"name"`
	Authorization []PermissionLevel `json:"authorization"`
	Data          BytesType         `json:"data"`
}

type ActionReceipt struct {
	Receiver       AccountName       `json:"receiver"`
	ActDigest      Sha256Type        `json:"act_digest"`
	GlobalSequence uint64            `json:"global_sequence"` ///< total number of actions dispatched since genesis
	RecvSequence   uint64            `json:"recv_sequence"`   ///< total number of actions with this receiver since genesis
	AuthSequence   map[string]uint64 `json:"auth_sequence"`
	CodeSequence   uint              `json:"code_sequence"` ///< total number of setcodes
	AbiSequence    uint              `json:"abi_sequence"`  ///< total number of setabis
}

type BaseActionTrace struct {
	Receipt       ActionReceipt `json:"receipt"`
	Act           Action        `json:"act"`
	Elapsed       TimePoint     `json:"elapsed"`
	CpuUsage      uint64        `json:"cpu_usage"`
	Console       string        `json:"console"`
	TotalCpuUsage uint64        `json:"total_cpu_usage"` /// total of inline_traces[x].cpu_usage + cpu_usage
	TrxId         Sha256Type    `json:"trx_id"`          ///< the transaction that generated this action
}

type ActionTrace struct {
	BaseActionTrace
	InlineTraces []ActionTrace `json:"inline_traces"`
}
