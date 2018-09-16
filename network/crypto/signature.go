package crypto

import (
	"bytes"
	"encoding/json"
	"github.com/btcsuite/btcd/btcec"
	"github.com/btcsuite/btcutil/base58"
	"github.com/pkg/errors"
	"strings"
)

const SignatureBasePrefix = "SIG_"

type Signature struct {
	Prefix Prefix
	Data   []byte
}

func NewEmptySignature() *Signature {
	return &Signature{
		Prefix: PrefixK1,
		Data:   make([]byte, 65, 65),
	}
}

func NewSignature(input string) (Signature, error) {
	if !strings.HasPrefix(input, SignatureBasePrefix) {
		return Signature{}, errors.New("input signature prefix error")
	}
	if len(input) < 8 {
		return Signature{}, errors.New("input signature error")
	}
	input = input[4:]

	var prefix Prefix
	var pre = input[:3]
	switch pre {
	case "K1_":
		prefix = PrefixK1
	case "R1_":
		prefix = PrefixR1
	default:
		return Signature{}, errors.New("signature prefix error")
	}
	input = input[3:]

	decoded := base58.Decode(input)
	content := decoded[:len(decoded)-4]
	checksum := decoded[len(decoded)-4:]
	ripemdChecksum := ripemd160Checksum(content)
	if !bytes.Equal(ripemdChecksum, checksum) {
		return Signature{}, errors.New("checksum error")
	}

	return Signature{Prefix: prefix, Data: content}, nil
}

func (s Signature) IsEmpty() bool {
	e := make([]byte, 65, 65)
	if bytes.Equal(s.Data, e) {
		return true
	}
	return false
}

func (s Signature) String() string {
	checksum := ripemd160Checksum(s.Data)
	buf := append(s.Data[:], checksum...)
	return SignatureBasePrefix + s.Prefix.StringPrefix() + base58.Encode(buf)
}

func (s Signature) MarshalJSON() ([]byte, error) {
	return json.Marshal(s.String())
}

func (s *Signature) UnmarshalJSON(data []byte) (err error) {
	var str string
	err = json.Unmarshal(data, &str)
	if err != nil {
		return
	}
	*s, err = NewSignature(str)
	return
}

func (s Signature) PublicKey(hash []byte) (key PublicKey, err error) {
	if s.Prefix != PrefixK1 {
		return key, errors.New("signature prefix error")
	}

	recoveredKey, _, err := btcec.RecoverCompact(btcec.S256(), s.Data, hash)
	if err != nil {
		return key, err
	}
	key = PublicKey{s.Prefix, recoveredKey.SerializeCompressed()}

	return key, nil
}
