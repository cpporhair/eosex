package network

import (
	"bytes"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"eosex/network/crypto"
	"errors"
	"fmt"
	"math"
	"strconv"
	"strings"
	"time"
)

var TypeParseSize = struct {
	BoolSize           int
	ByteSize           int
	Int8Size           int
	UInt16Size         int
	Int16Size          int
	UInt32Size         int
	UInt64Size         int
	TstampSize         int
	BlockTimestampSize int
	Sha256TypeSize     int
	PublicKeySize      int
	SignatureSize      int
}{
	BoolSize:           1,
	ByteSize:           1,
	Int8Size:           1,
	UInt16Size:         2,
	Int16Size:          2,
	UInt32Size:         4,
	UInt64Size:         8,
	TstampSize:         8,
	BlockTimestampSize: 4,
	Sha256TypeSize:     32,
	PublicKeySize:      34,
	SignatureSize:      66,
}

type Uvarint32 uint32

type Name string
type AccountName Name
type PermissionName Name
type ActionName Name
type TableName Name
type ScopeName Name

type TypeName string
type FieldName string

type TypeDef struct {
	NewTypeName TypeName `json:"new_type_name"`
	Type        TypeName `json:"type"`
}

type StructDef struct {
	Name   string     `json:"name"`
	Base   string     `json:"base"`
	Fields []FieldDef `json:"fields,omitempty"`
}

type FieldDef struct {
	Name string `json:"name"`
	Type string `json:"type"`
}

type ActionDef struct {
	Name              ActionName `json:"name"`
	Type              string     `json:"type"`
	RicardianContract string     `json:"ricardian_contract"`
}

type TableDef struct {
	Name      TableName `json:"name"`
	IndexType string    `json:"index_type"`
	KeyNames  []string  `json:"key_names,omitempty"`
	KeyTypes  []string  `json:"key_types,omitempty"`
	Type      string    `json:"type"`
}

type ClausePair struct {
	ID   string `json:"id"`
	Body string `json:"body"`
}

type ErrorMessage struct {
	ErrorCode uint64 `json:"error_code"`
	ErrorMsg  string `json:"error_msg"`
}

type ABIDef struct {
	Version          string         `json:"version"`
	Types            []TypeDef      `json:"types"`
	Structs          []StructDef    `json:"structs"`
	Actions          []ActionDef    `json:"actions"`
	Tables           []TableDef     `json:"tables"`
	RicardianClauses []ClausePair   `json:"ricardian_clauses"`
	ErrorMessages    []ErrorMessage `json:"error_messages"`
	AbiExtensions    ExtensionsType `json:"abi_extensions"`
}

type SetCode struct {
	Account   AccountName `json:"account"`
	VMType    uint8       `json:"vmtype"`
	VMVersion uint8       `json:"vmversion"`
	Code      BytesType   `json:"code"`
}

type SetABI struct {
	Account AccountName `json:"account"`
	ABI     BytesType   `json:"abi"`
}

type AccountResourceLimit struct {
	Used      int64 `json:"used"`
	Available int64 `json:"available"`
	Max       int64 `json:"max"`
}

type DelegatedBandwidth struct {
	From      AccountName `json:"from"`
	To        AccountName `json:"to"`
	NetWeight Asset       `json:"net_weight"`
	CPUWeight Asset       `json:"cpu_weight"`
}

type TotalResources struct {
	Owner     AccountName `json:"owner"`
	NetWeight Asset       `json:"net_weight"`
	CPUWeight Asset       `json:"cpu_weight"`
	RAMBytes  int64       `json:"ram_bytes"`
}

type VoterInfo struct {
	Owner             AccountName   `json:"owner"`
	Proxy             AccountName   `json:"proxy"`
	Producers         []AccountName `json:"producers"`
	Staked            int64         `json:"staked"`
	LastVoteWeight    float64       `json:"last_vote_weight"`
	ProxiedVoteWeight float64       `json:"proxied_vote_weight"`
	IsProxy           byte          `json:"is_proxy"`
}

type RefundRequest struct {
	Owner       AccountName  `json:"owner"`
	RequestTime TimePointSec `json:"request_time"`
	NetAmount   Asset        `json:"net_amount"`
	CPUAmount   Asset        `json:"cpu_amount"`
}

type AssetDef struct {
	MaxAmount int64 `json:"max_amount"`
	Amount    int64 `json:"amount"`
	Symbol    `json:"sym"`
}

// Asset
type Asset struct {
	MaxAmount  int64 `json:"max_amount"`
	Amount     int64 `json:"amount"`
	SymbolType `json:"sym"`
}

func (a Asset) Add(other Asset) Asset {
	if a.SymbolType != other.SymbolType {
		panic("Add applies only to assets with the same symbol")
	}
	return Asset{Amount: a.Amount + other.Amount, SymbolType: a.SymbolType}
}

func (a Asset) Sub(other Asset) Asset {
	if a.SymbolType != other.SymbolType {
		panic("Sub applies only to assets with the same symbol")
	}
	return Asset{Amount: a.Amount - other.Amount, SymbolType: a.SymbolType}
}

func (a Asset) String() string {
	strInt := fmt.Sprintf("%d", a.Amount)
	if len(strInt) < int(a.SymbolType.Precision+1) {
		// prepend `0` for the difference:
		strInt = strings.Repeat("0", int(a.SymbolType.Precision+uint8(1))-len(strInt)) + strInt
	}

	var result string
	if a.SymbolType.Precision == 0 {
		result = strInt
	} else {
		result = strInt[:len(strInt)-int(a.SymbolType.Precision)] + "." + strInt[len(strInt)-int(a.SymbolType.Precision):]
	}

	return fmt.Sprintf("%s %s", result, a.SymbolType.Symbol)
}

type Symbol struct {
	MaxPrecision uint8  `json:"max_precision"`
	Precision    uint8  `json:"precision"`
	Value        uint64 `json:"m_value"`
}

type SymbolType struct {
	Precision uint8
	Symbol    string
}

var EOSSymbol = SymbolType{Precision: 4, Symbol: "EOS"}

func NewEOSAssetFromString(amount string) (out Asset, err error) {
	if len(amount) == 0 {
		return out, fmt.Errorf("cannot be an empty string")
	}

	if strings.Contains(amount, " EOS") {
		amount = strings.Replace(amount, " EOS", "", 1)
	}
	if !strings.Contains(amount, ".") {
		val, err := strconv.ParseInt(amount, 10, 64)
		if err != nil {
			return out, err
		}
		return NewEOSAsset(val * 10000), nil
	}

	parts := strings.Split(amount, ".")
	if len(parts) != 2 {
		return out, fmt.Errorf("cannot have two . in amount")
	}

	if len(parts[1]) > 4 {
		return out, fmt.Errorf("EOS has only 4 decimals")
	}

	val, err := strconv.ParseInt(strings.Replace(amount, ".", "", 1), 10, 64)
	if err != nil {
		return out, err
	}
	return NewEOSAsset(val * int64(math.Pow10(4-len(parts[1])))), nil
}

func NewEOSAsset(amount int64) Asset {
	return Asset{Amount: amount, SymbolType: EOSSymbol}
}

func NewAsset(in string) (out Asset, err error) {
	sec := strings.SplitN(in, " ", 2)
	if len(sec) != 2 {
		return out, fmt.Errorf("invalid format %q, expected an amount and a currency symbol", in)
	}

	if len(sec[1]) > 7 {
		return out, fmt.Errorf("currency symbol %q too long", sec[1])
	}

	out.SymbolType.Symbol = sec[1]
	amount := sec[0]
	amountSec := strings.SplitN(amount, ".", 2)

	if len(amountSec) == 2 {
		out.SymbolType.Precision = uint8(len(amountSec[1]))
	}

	val, err := strconv.ParseInt(strings.Replace(amount, ".", "", 1), 10, 64)
	if err != nil {
		return out, err
	}

	out.Amount = val

	return
}

func (a *Asset) UnmarshalJSON(data []byte) error {
	var s string
	err := json.Unmarshal(data, &s)
	if err != nil {
		return err
	}

	asset, err := NewAsset(s)
	if err != nil {
		return err
	}

	*a = asset

	return nil
}

func (a Asset) MarshalJSON() (data []byte, err error) {
	return json.Marshal(a.String())
}

type Permission struct {
	PermName     string    `json:"perm_name"`
	Parent       string    `json:"parent"`
	RequiredAuth Authority `json:"required_auth"`
}

type PermissionLevel struct {
	Actor      AccountName    `json:"actor"`
	Permission PermissionName `json:"permission"`
}

type PermissionLevelWeight struct {
	Permission PermissionLevel `json:"permission"`
	Weight     uint16          `json:"weight"`
}

type Authority struct {
	Threshold uint32                  `json:"threshold"`
	Keys      []KeyWeight             `json:"keys,omitempty"`
	Accounts  []PermissionLevelWeight `json:"accounts,omitempty"`
	Waits     []WaitWeight            `json:"waits,omitempty"`
}

type KeyWeight struct {
	PublicKey crypto.PublicKey `json:"key"`
	Weight    uint16           `json:"weight"`
}

type WaitWeight struct {
	WaitSec uint32 `json:"wait_sec"`
	Weight  uint16 `json:"weight"`
}

type GetCodeResp struct {
	AccountName AccountName `json:"account_name"`
	CodeHash    string      `json:"code_hash"`
	WASM        string      `json:"wasm"`
	ABI         ABIDef      `json:"abi"`
}

type GetABIResp struct {
	AccountName AccountName `json:"account_name"`
	ABI         ABIDef      `json:"abi"`
}

// Tstamp
type Tstamp struct {
	time.Time
}

const TstampFormat = "2006-01-02T15:04:05"

func (t Tstamp) MarshalJSON() ([]byte, error) {
	return json.Marshal(fmt.Sprintf("%d", t.UnixNano()))
}

func (t *Tstamp) UnmarshalJSON(data []byte) (err error) {
	var unixNano int64
	if data[0] == '"' {
		var s string
		if err = json.Unmarshal(data, &s); err != nil {
			return
		}
		unixNano, err = strconv.ParseInt(s, 10, 64)
		if err != nil {
			return err
		}
	} else {
		unixNano, err = strconv.ParseInt(string(data), 10, 64)
		if err != nil {
			return err
		}
	}
	*t = Tstamp{time.Unix(0, unixNano)}

	return nil
}

func (t Tstamp) Add(a Tstamp) Tstamp {
	n := t.UnixNano() + a.UnixNano()
	return Tstamp{time.Unix(0, n)}
}

type BlockTimestamp struct {
	time.Time
}

const BlockTimestampFormat = "2006-01-02T15:04:05.000"

func (t BlockTimestamp) MarshalJSON() ([]byte, error) {
	return []byte(fmt.Sprintf("%q", t.Format(BlockTimestampFormat))), nil
}

func (t *BlockTimestamp) UnmarshalJSON(data []byte) (err error) {
	if len(data) == 0 {
		return errors.New("data is empty")
	}
	t.Time, err = time.Parse(BlockTimestampFormat, string(data))
	return err
}

type TimePoint struct {
	time.Time
}

const TimePointFormat = "2006-01-02T15:04:05.000"

func (t TimePoint) MarshalJSON() ([]byte, error) {
	return []byte(fmt.Sprintf("%q", t.Format(TimePointFormat))), nil
}

func (t *TimePoint) UnmarshalJSON(data []byte) (err error) {
	if len(data) == 0 {
		return errors.New("data is empty")
	}
	t.Time, err = time.Parse(`"`+TimePointFormat+`"`, string(data))
	return err
}

// TimePointSec
type TimePointSec struct {
	time.Time
}

func (t TimePointSec) MarshalJSON() ([]byte, error) {
	return []byte(fmt.Sprintf("%q", t.Format(TimePointFormat))), nil
}

func (t *TimePointSec) UnmarshalJSON(data []byte) (err error) {
	if len(data) == 0 {
		return errors.New("data is empty")
	}
	t.Time, err = time.Parse(TimePointFormat, string(data))
	return err
}

// BytesType
type BytesType []byte

func (t BytesType) MarshalJSON() ([]byte, error) {
	return json.Marshal(hex.EncodeToString(t))
}

func (t *BytesType) UnmarshalJSON(data []byte) (err error) {
	var s string
	err = json.Unmarshal(data, &s)
	if err != nil {
		return
	}

	*t, err = hex.DecodeString(s)
	return
}

// Sha256Type
type Sha256Type []byte //  32 bytes

func NewEmptySha256Type() Sha256Type {
	s := make([]byte, TypeParseSize.Sha256TypeSize)
	return s
}

func (t Sha256Type) String() string {
	return hex.EncodeToString(t)
}

func (t Sha256Type) MarshalJSON() ([]byte, error) {
	return json.Marshal(hex.EncodeToString(t))
}

func (t *Sha256Type) UnmarshalJSON(data []byte) (err error) {
	var s string
	err = json.Unmarshal(data, &s)
	if err != nil {
		return
	}
	*t, err = hex.DecodeString(s)
	return
}

func Sh256Hash(v []byte) []byte {
	h := sha256.New()
	_, _ = h.Write(v)
	return h.Sum(nil)
}

func (t Sha256Type) IsEmpty() bool {
	if len(t) != TypeParseSize.Sha256TypeSize {
		return true
	}
	if bytes.Equal(t, NewEmptySha256Type()) {
		return true
	}
	return false
}

func (t Sha256Type) Equal(s Sha256Type) bool {
	if bytes.Equal(t, s) {
		return true
	}
	return false
}

type BlockIdType Sha256Type
type ChecksumType Sha256Type
type Checksum256Type Sha256Type
type TransactionIdType ChecksumType

//std::string itoh(I n, size_t hlen = sizeof(I)<<1) {
//static const char* digits = "0123456789abcdef";
//std::string r(hlen, '0');
//for(size_t i = 0, j = (hlen - 1) * 4 ; i < hlen; ++i, j -= 4)
//r[i] = digits[(n>>j) & 0x0f];
//return r;
//}
