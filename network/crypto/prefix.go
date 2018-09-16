package crypto

type Prefix uint8

const (
	PrefixK1 = Prefix(iota)
	PrefixR1
)

func (p Prefix) String() string {
	switch p {
	case PrefixK1:
		return "K1"
	case PrefixR1:
		return "R1"
	default:
		return "UN" // unknown
	}
}

func (p Prefix) StringPrefix() string {
	return p.String() + "_"
}
