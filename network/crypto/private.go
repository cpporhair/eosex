package crypto

import (
	"encoding/json"
	"github.com/eoscanada/eos-go/ecc"
)

const PrivateKeyBasePrefix = "PVT_"

type PrivateKey struct {
	eccPrivKey *ecc.PrivateKey
}

func NewPrivateKey(input string) (*PrivateKey, error) {
	eccPriv, err := ecc.NewPrivateKey(input)
	if err != nil {
		return nil, err
	}
	return &PrivateKey{
		eccPrivKey: eccPriv,
	}, nil
}

func (p *PrivateKey) PublicKey() PublicKey {
	eccPubl := p.eccPrivKey.PublicKey()
	publicKey := PublicKey{
		Prefix: Prefix(eccPubl.Curve),
		Data:   eccPubl.Content,
	}
	return publicKey
}

func (p *PrivateKey) Sign(hash []byte) (*Signature, error) {
	eccSign, err := p.eccPrivKey.Sign(hash)
	if err != nil {
		return nil, err
	}
	sign := &Signature{Prefix: Prefix(eccSign.Curve), Data: eccSign.Content}
	return sign, nil
}

func (p *PrivateKey) String() string {
	return p.eccPrivKey.String()
}

func (p *PrivateKey) MarshalJSON() ([]byte, error) {
	return json.Marshal(p.String())
}

func (p *PrivateKey) UnmarshalJSON(v []byte) (err error) {
	var s string
	if err = json.Unmarshal(v, &s); err != nil {
		return
	}

	newPrivKey, err := NewPrivateKey(s)
	if err != nil {
		return
	}

	*p = *newPrivKey

	return
}
