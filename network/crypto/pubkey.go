package crypto

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"github.com/btcsuite/btcd/btcec"
	"github.com/btcsuite/btcutil/base58"
	"golang.org/x/crypto/ripemd160"
	"strings"
)

const PublicKeyBasePrefix = "PUB_"
const PublicKeyLegacyPrefix = "EOS"

type PublicKey struct {
	Prefix Prefix
	Data   []byte
}

func NewEmptyPublicKey() *PublicKey {
	return &PublicKey{
		Prefix: PrefixK1,
		Data:   make([]byte, 33, 33),
	}
}

func NewPublicKey(input string) (key PublicKey, err error) {
	if len(input) < 8 {
		return key, errors.New("input key error")
	}
	var prefix Prefix
	var keyContent string
	if strings.HasPrefix(input, PublicKeyBasePrefix) {
		keyContent = input[len(PublicKeyBasePrefix):]

		pre := keyContent[:3]
		switch pre {
		case "K1_":
			prefix = PrefixK1
		case "R1_":
			prefix = PrefixR1
		default:
			return key, errors.New("key prefix error")
		}
		keyContent = keyContent[3:]
	} else if strings.HasPrefix(input, PublicKeyLegacyPrefix) {
		keyContent = input[len(PublicKeyLegacyPrefix):]
		prefix = PrefixK1
	} else {
		return key, errors.New("key prefix error")
	}

	decoded, err := base58Decode(keyContent)
	if err != nil {
		return key, fmt.Errorf("base58Decode: %s", err)
	}

	return PublicKey{Prefix: prefix, Data: decoded}, nil
}

func base58Decode(input string) (result []byte, err error) {
	decoded := base58.Decode(input)
	if len(decoded) < 5 {
		return nil, errors.New("base58 error")
	}
	var checksum [4]byte
	copy(checksum[:], decoded[len(decoded)-4:])
	if !bytes.Equal(ripemd160Checksum(decoded[:len(decoded)-4]), checksum[:]) {
		return nil, errors.New("checksum error")
	}
	d := decoded[:len(decoded)-4]
	result = append(result, d...)
	return
}

func ripemd160Checksum(input []byte) []byte {
	h := ripemd160.New()
	_, _ = h.Write(input)
	sum := h.Sum(nil)
	return sum[:4]
}

func ripemd160checksumWithPrefix(input []byte, prefix Prefix) []byte {
	h := ripemd160.New()
	_, _ = h.Write(input)
	_, _ = h.Write([]byte(prefix.String()))
	sum := h.Sum(nil)
	return sum[:4]
}

func (p PublicKey) Key() (*btcec.PublicKey, error) {
	key, err := btcec.ParsePubKey(p.Data, btcec.S256())
	if err != nil {
		return nil, fmt.Errorf("parsePubKey: %s", err)
	}
	return key, nil
}

func (p PublicKey) String() string {
	hash := ripemd160Checksum(p.Data)
	pubkey := append(p.Data, hash[:4]...)
	return PublicKeyLegacyPrefix + base58.Encode(pubkey)
}

func (p PublicKey) MarshalJSON() ([]byte, error) {
	s := p.String()
	return json.Marshal(s)
}

func (p *PublicKey) UnmarshalJSON(data []byte) error {
	var s string
	err := json.Unmarshal(data, &s)
	if err != nil {
		return err
	}
	key, err := NewPublicKey(s)
	if err != nil {
		return err
	}
	*p = key
	return nil
}

func (p PublicKey) Equal(key *PublicKey) bool {
	if key == nil || p.Prefix != key.Prefix {
		return false
	}
	if !bytes.Equal(p.Data, key.Data) {
		return false
	}
	return true
}
