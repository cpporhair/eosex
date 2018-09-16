package config

const (
	MessageHeaderSize uint   = 4
	NetVersionBase    uint16 = 0x04b5
	NetVersionRange   uint16 = 106
	NetVersion        uint16 = ProtoExplicitSync
	AppVersion        uint64 = 0x60947c0c

	ProtoBase           uint16 = 0
	ProtoExplicitSync          = 1
	LargeMsgNotify             = false
	MaxImplicitRequest  uint32 = 1500
	BlockIntervalMs     uint64 = 500
	BlockTimestampEpoch uint64 = 946684800000
	SyncFetchSpan       uint32 = 100

	//jungle.cryptolions.io chain_id
	ChainId = "038f4b0fc8ff18a4f0842a8f0564611f6e96e8535901dd45e43ac8691a1c4dca"
	//ChainId = "cf057bbfb72640471fd910bcb67639c22df9f92470936cddc1ade0e2f2e7dc4f"
)

var HttpListenEndpoint = ":8989"

var P2PListenEndpoint = "127.0.0.1:9898"

//var SuppliedPeers = []string{"127.0.0.1:9876"}
var SuppliedPeers = []string{"jungle.cryptolions.io:19876"}

var HttpServerAddr = "http://jungle.cryptolions.io:38888"

//var HttpServerAddr = "http://127.0.0.1:8888"

// Can be 'any' or 'producers' or 'specified' or 'none'. If 'specified', peer-key must be specified at least once.
// If only 'producers', peer-key is not required. 'producers' and 'specified' may be combined.
var AllowedConnection = []string{"any"}
var PeerKey = []string{}
