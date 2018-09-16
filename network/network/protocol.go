package network

import (
	"crypto/sha256"
	"encoding/binary"
	"encoding/hex"
	"encoding/json"
	"eosex/network/crypto"
	"fmt"
	"time"
)

type Message interface {
	Serialize() []byte
}

type MessageData struct {
	data []byte
}

func NewMessageData(data []byte) *MessageData {
	msg := &MessageData{
		data: data,
	}
	return msg
}

func (m *MessageData) Serialize() []byte {
	return m.data
}

func (m *MessageData) GetLength() uint32 {
	return binary.LittleEndian.Uint32(m.data[0:4])
}

func (m *MessageData) GetBody() []byte {
	return m.data[4:]
}

type NetMessageType byte

const (
	HandshakeMessageType NetMessageType = iota
	ChainSizeType
	GoAwayMessageType
	TimeMessageType
	NoticeMessageType
	RequestMessageType
	SyncRequestMessageType
	SignedBlockType
	PackedTransactionType
)

func (t NetMessageType) CreateNetMessage() NetMessage {
	switch t {
	case HandshakeMessageType:
		return new(HandshakeMessage)
	case GoAwayMessageType:
		return new(GoAwayMessage)
	case ChainSizeType:
		return new(ChainSizeMessage)
	case TimeMessageType:
		return new(TimeMessage)
	case NoticeMessageType:
		return new(NoticeMessage)
	case RequestMessageType:
		return new(RequestMessage)
	case SyncRequestMessageType:
		return new(SyncRequestMessage)
	case SignedBlockType:
		return new(SignedBlock)
	case PackedTransactionType:
		return new(PackedTransaction)
	default:
		return nil
	}
}

type NetMessageInfo struct {
	Length  uint32         `json:"length"`
	Type    NetMessageType `json:"type"`
	Payload []byte         `json:"-"`
	NetMsg  NetMessage     `json:"message" eos:"-"`
	Raw     []byte         `json:"-"`
}

type NetMessage interface {
	GetType() NetMessageType
}

type SendMessage struct {
	Length uint32
	Type   NetMessageType
	NetMsg NetMessage
}

type ChainSizeMessage struct {
	LastIrreversibleBlockNum uint32     `json:"last_irreversible_block_num"`
	LastIrreversibleBlockID  Sha256Type `json:"last_irreversible_block_id"`
	HeadNum                  uint32     `json:"head_num"`
	HeadID                   Sha256Type `json:"head_id"`
}

func (m *ChainSizeMessage) Serialize() []byte {
	data, err := MarshalBinary(m)
	if err != nil {
		return nil
	}
	return data
}

func (m *ChainSizeMessage) GetType() NetMessageType {
	return ChainSizeType
}

type HandshakeMessage struct {
	// net_plugin/protocol.hpp handshake_message
	NetworkVersion           uint16           `json:"network_version"`
	ChainID                  Sha256Type       `json:"chain_id"`
	NodeID                   Sha256Type       `json:"node_id"`
	Key                      crypto.PublicKey `json:"key"`
	Time                     Tstamp           `json:"time"`
	Token                    Sha256Type       `json:"token"`
	Signature                crypto.Signature `json:"sig"`
	P2PAddress               string           `json:"p2p_address"`
	LastIrreversibleBlockNum uint32           `json:"last_irreversible_block_num"`
	LastIrreversibleBlockID  Sha256Type       `json:"last_irreversible_block_id"`
	HeadNum                  uint32           `json:"head_num"`
	HeadID                   Sha256Type       `json:"head_id"`
	OS                       string           `json:"os"`
	Agent                    string           `json:"agent"`
	Generation               uint16           `json:"generation"`
}

func (m *HandshakeMessage) GetType() NetMessageType {
	return HandshakeMessageType
}

func (m *HandshakeMessage) Serialize() []byte {
	data, err := MarshalBinary(m)
	if err != nil {
		return nil
	}
	return data
}

type GoAwayReason uint8

const (
	GoAwayNoReason GoAwayReason = iota
	GoAwaySelf
	GoAwayDuplicate
	GoAwayWrongChain
	GoAwayWrongVersion
	GoAwayForked
	GoAwayUnlinkable
	GoAwayBadTransaction
	GoAwayValidation
	GoAwayBenignOther
	GoAwayFatalOther
	GoAwayAuthentication
)

func (r GoAwayReason) ReasonStr() string {
	switch r {
	case GoAwayNoReason:
		return "no reason"
	case GoAwaySelf:
		return "self connect"
	case GoAwayDuplicate:
		return "duplicate"
	case GoAwayWrongChain:
		return "wrong chain"
	case GoAwayWrongVersion:
		return "wrong version"
	case GoAwayForked:
		return "chain is forked"
	case GoAwayUnlinkable:
		return "unlinkable block received"
	case GoAwayBadTransaction:
		return "bad transaction"
	case GoAwayValidation:
		return "invalid block"
	case GoAwayBenignOther:
		return "some other non-fatal condition"
	case GoAwayFatalOther:
		return "some other failure"
	case GoAwayAuthentication:
		return "authentication failure"
	default:
		return "some crazy reason"
	}
}

type GoAwayMessage struct {
	Reason GoAwayReason `json:"reason"`
	NodeID Sha256Type   `json:"node_id"`
}

func (m *GoAwayMessage) Serialize() []byte {
	data, err := MarshalBinary(m)
	if err != nil {
		return nil
	}
	return data
}

func (m *GoAwayMessage) GetType() NetMessageType {
	return GoAwayMessageType
}

func NewGoAwayMessage(r GoAwayReason) *GoAwayMessage {
	return &GoAwayMessage{
		Reason: r,
		NodeID: NewEmptySha256Type(),
	}
}

type TimeMessage struct {
	Origin      Tstamp `json:"org"` //!< origin timestamp
	Receive     Tstamp `json:"rec"` //!< receive timestamp
	Transmit    Tstamp `json:"xmt"` //!< transmit timestamp
	Destination Tstamp `json:"dst"` //!< destination timestamp
}

func (m *TimeMessage) Serialize() []byte {
	data, err := MarshalBinary(m)
	if err != nil {
		return nil
	}
	return data
}

func (m *TimeMessage) GetType() NetMessageType {
	return TimeMessageType
}

func (m *TimeMessage) String() string {
	return fmt.Sprintf("org: %s, rec: %s, xmt: %s, dst: %s", m.Origin, m.Receive, m.Transmit, m.Destination)
}

type IDListModes int32

const (
	None IDListModes = iota
	CatchUp
	LastIrrCatchUp
	Normal
)

func (m IDListModes) ModesStr() string {
	switch m {
	case None:
		return "none"
	case CatchUp:
		return "catch up"
	case LastIrrCatchUp:
		return "last irreversible"
	case Normal:
		return "normal"
	default:
		return "undefined mode"
	}
}

type OrderedTransactionIDs struct {
	Mode    IDListModes  `json:"mode"`
	Pending uint32       `json:"pending"`
	IDs     []Sha256Type `json:"ids"`
}

type OrderedBlockIDs struct {
	Mode    IDListModes  `json:"mode"`
	Pending uint32       `json:"pending"`
	IDs     []Sha256Type `json:"ids"`
}

type NoticeMessage struct {
	KnownTrx    OrderedTransactionIDs `json:"known_trx"`
	KnownBlocks OrderedBlockIDs       `json:"known_blocks"`
}

func (m *NoticeMessage) Serialize() []byte {
	data, err := MarshalBinary(m)
	if err != nil {
		return nil
	}
	return data
}

func (m *NoticeMessage) GetType() NetMessageType {
	return NoticeMessageType
}

type RequestMessage struct {
	ReqTrx    OrderedTransactionIDs `json:"req_trx"`
	ReqBlocks OrderedBlockIDs       `json:"req_blocks"`
}

func (m *RequestMessage) Serialize() []byte {
	data, err := MarshalBinary(m)
	if err != nil {
		return nil
	}
	return data
}

func (m *RequestMessage) GetType() NetMessageType {
	return RequestMessageType
}

type SyncRequestMessage struct {
	StartBlock uint32 `json:"start_block"`
	EndBlock   uint32 `json:"end_block"`
}

//
//type SliceMock struct {
//	addr uintptr
//	len  int
//	cap  int
//}

//func (m *SyncRequestMessage) Serialize() []byte {
//	Len := unsafe.Sizeof(*m)
//	headerBytes := &SliceMock{
//		addr: uintptr(unsafe.Pointer(m)),
//		cap:  int(Len),
//		len:  int(Len),
//	}
//	data := *(*[]byte)(unsafe.Pointer(headerBytes))
//	return data
//}

func (m *SyncRequestMessage) Serialize() []byte {
	data, err := MarshalBinary(m)
	if err != nil {
		return nil
	}
	return data
}

func (m *SyncRequestMessage) GetType() NetMessageType {
	return SyncRequestMessageType
}

func (m *SyncRequestMessage) String() string {
	return fmt.Sprintf("SyncRequest: Start Block [%d] End Block [%d]", m.StartBlock, m.EndBlock)
}

type TransactionStatus uint8

const (
	TransactionStatusExecuted TransactionStatus = iota ///< succeed, no error handler executed
	TransactionStatusSoftFail                          ///< objectively failed (not executed), error handler executed
	TransactionStatusHardFail                          ///< objectively failed and error handler objectively failed thus no state change
	TransactionStatusDelayed                           ///< transaction delayed/deferred/scheduled for future execution
	TransactionStatusExpired                           ///< transaction expired and storage space refuned to user
)

type TransactionReceiptHeader struct {
	Status               TransactionStatus `json:"status"`
	CPUUsageMicroSeconds uint32            `json:"cpu_usage_us"`
	NetUsageWords        Uvarint32         `json:"net_usage_words"`
}

type TransactionWithID struct {
	ID     Sha256Type
	Packed *PackedTransaction
}

type TransactionReceipt struct {
	TransactionReceiptHeader
	Transaction TransactionWithID `json:"trx"`
}

type ProducerKey struct {
	ProducerName    AccountName      `json:"producer_name"`
	BlockSigningKey crypto.PublicKey `json:"block_signing_key"`
}

type ProducerScheduleType struct {
	Version   uint32        `json:"version"`
	Producers []ProducerKey `json:"producers"`
}

type BlockHeader struct {
	Timestamp        BlockTimestamp        `json:"timestamp"`
	Producer         AccountName           `json:"producer"`
	Confirmed        uint16                `json:"confirmed"`
	Previous         Sha256Type            `json:"previous"`
	TransactionMRoot Sha256Type            `json:"transaction_mroot"`
	ActionMRoot      Sha256Type            `json:"action_mroot"`
	ScheduleVersion  uint32                `json:"schedule_version"`
	NewProducers     *ProducerScheduleType `json:"new_producers" eos:"optional"`
	HeaderExtensions ExtensionsType        `json:"header_extensions"`
}

func (b *BlockHeader) BlockNumber() uint32 {
	return binary.BigEndian.Uint32(b.Previous[:4]) + 1
}

func BlockNum(blockID string) uint32 {
	if len(blockID) < 8 {
		return 0
	}
	bn, err := hex.DecodeString(blockID[:8])
	if err != nil {
		return 0
	}
	return binary.BigEndian.Uint32(bn)
}

func (b *BlockHeader) BlockID() (Sha256Type, error) {
	mb, err := MarshalBinary(b)
	if err != nil {
		return nil, err
	}
	h := sha256.New()
	_, _ = h.Write(mb)
	sum := h.Sum(nil)

	binary.BigEndian.PutUint32(sum, b.BlockNumber())

	return sum, nil
}

type SignedBlockHeader struct {
	BlockHeader
	ProducerSignature crypto.Signature `json:"producer_signature"`
}

type SignedBlock struct {
	SignedBlockHeader
	Transactions    []TransactionReceipt `json:"transactions"`
	BlockExtensions []*Extension         `json:"block_extensions"`
}

func (m *SignedBlock) Serialize() []byte {
	data, err := MarshalBinary(m)
	if err != nil {
		return nil
	}
	return data
}

func (m *SignedBlock) GetType() NetMessageType {
	return SignedBlockType
}

func (m *SignedBlock) String() string {
	return "SignedBlock"
}

type Extension struct {
	Type uint16    `json:"type"`
	Data BytesType `json:"data"`
}

type ExtensionsType []Extension

type TransactionHeader struct {
	Expiration       TimePointSec `json:"expiration"`
	RefBlockNum      uint16       `json:"ref_block_num"`
	RefBlockPrefix   uint32       `json:"ref_block_prefix"`
	MaxNetUsageWords Uvarint32    `json:"max_net_usage_words"`
	MaxCPUUsageMS    uint8        `json:"max_cpu_usage_ms"`
	DelaySec         Uvarint32    `json:"delay_sec"`
}

type Transaction struct {
	TransactionHeader
	ContextFreeActions []*Action      `json:"context_free_actions"`
	Actions            []*Action      `json:"actions"`
	Extensions         ExtensionsType `json:"transaction_extensions"`
}

func (tx *Transaction) SetExpiration(in time.Duration) {
	tx.Expiration = TimePointSec{time.Now().UTC().Add(in)}
}

type SignedTransaction struct {
	*Transaction
	Signatures      []crypto.Signature `json:"signatures"`
	ContextFreeData []BytesType        `json:"context_free_data"`
	//packed *PackedTransaction
}

func NewSignedTransaction(tx *Transaction) *SignedTransaction {
	return &SignedTransaction{
		Transaction:     tx,
		Signatures:      make([]crypto.Signature, 0),
		ContextFreeData: make([]BytesType, 0),
	}
}

func (s *SignedTransaction) String() string {

	data, err := json.Marshal(s)
	if err != nil {
		return err.Error()
	}
	return string(data)
}

type CompressionType uint8

const (
	CompressionNone = CompressionType(iota)
	CompressionZlib
)

func (c CompressionType) String() string {
	switch c {
	case CompressionNone:
		return "none"
	case CompressionZlib:
		return "zlib"
	default:
		return ""
	}
}

func (c CompressionType) MarshalJSON() ([]byte, error) {
	return json.Marshal(c.String())
}

func (c *CompressionType) UnmarshalJSON(data []byte) error {
	var s string
	err := json.Unmarshal(data, &s)
	if err != nil {
		return err
	}

	switch s {
	case "zlib":
		*c = CompressionZlib
	default:
		*c = CompressionNone
	}
	return nil
}

type PackedTransaction struct {
	Signatures            []crypto.Signature `json:"signatures"`
	Compression           CompressionType    `json:"compression"`
	PackedContextFreeData BytesType          `json:"packed_context_free_data"`
	PackedTrx             BytesType          `json:"packed_trx"`
	unpackedTrx           *Transaction
}

func (p *PackedTransaction) ID() Sha256Type {
	h := sha256.New()
	_, _ = h.Write(p.PackedTrx)
	return h.Sum(nil)
}

func (m *PackedTransaction) Serialize() []byte {
	data, err := MarshalBinary(m)
	if err != nil {
		return nil
	}
	return data
}

func (m *PackedTransaction) GetType() NetMessageType {
	return PackedTransactionType
}
