package network

import "strings"

func StringToName(s string) uint64 {
	var name uint64
	var i uint32
	sLen := uint32(len(s))

	for ; i < sLen && i < 12; i++ {
		name |= (charToSymbol(s[i]) & 0x1f) << (64 - 5*(i+1))
	}
	if i == 12 && sLen > 12 {
		name |= charToSymbol(s[12]) & 0x0f
	}

	return name
}

func charToSymbol(c byte) uint64 {
	if c >= 'a' && c <= 'z' {
		r := (c - 'a') + 6
		return uint64(r)
	}
	if c >= '1' && c <= '5' {
		r := (c - '1') + 1
		return uint64(r)
	}
	return 0
}

var charmap = []byte(".12345abcdefghijklmnopqrstuvwxyz")

func NameToString(value uint64) string {
	//str := strings.Repeat(".", 13)
	str := []byte{'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.'}

	tmp := value
	for i := uint32(0); i <= 12; i++ {
		bit := 0x1f
		if i == 0 {
			bit = 0x0f
		}
		c := charmap[tmp&uint64(bit)]
		str[12-i] = c

		shift := uint(5)
		if i == 0 {
			shift = 4
		}
		tmp >>= shift
	}
	return strings.TrimRight(string(str), ".")
}
