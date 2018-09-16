package network

import (
	"bytes"
	"encoding/binary"
	"eosex/network/config"
	"eosex/network/crypto"
	"errors"
	"io"
	"reflect"
)

type Packer struct {
	writer io.Writer
}

func NewPacker(w io.Writer) *Packer {
	return &Packer{
		writer: w,
	}
}

func (p *Packer) Pack(v interface{}) (err error) {
	switch vtype := v.(type) {
	case string:
		return p.writeString(vtype)
	case byte:
		return p.writeByte(vtype)
	case int8:
		return p.writeByte(byte(vtype))
	case int16:
		return p.writeInt16(vtype)
	case uint16:
		return p.writeUint16(vtype)
	case uint32:
		return p.writeUint32(vtype)
	case uint64:
		return p.writeUint64(vtype)
	case Uvarint32:
		return p.writeUVarInt(int(vtype))
	case bool:
		return p.writeBool(vtype)
	case BytesType:
		return p.writeByteArray(vtype)
	case []byte:
		return p.writeByteArray(vtype)
	case CompressionType:
		return p.writeByte(uint8(vtype))
	case TransactionStatus:
		return p.writeByte(uint8(vtype))
	case IDListModes:
		return p.writeUint32(uint32(vtype))
	case Sha256Type:
		return p.writeSha256Type(vtype)
	case crypto.PublicKey:
		return p.writePublicKey(vtype)
	case crypto.Signature:
		return p.writeSignature(vtype)
	case Tstamp:
		return p.writeTstamp(vtype)
	case BlockTimestamp:
		return p.writeBlockTimestamp(vtype)
	case TimePoint:
		return p.writeTimePoint(vtype)
	case TimePointSec:
		return p.writeTimePointSec(vtype)
	case Name:
		return p.writeName(vtype)
	case AccountName:
		name := Name(vtype)
		return p.writeName(name)
	case PermissionName:
		name := Name(vtype)
		return p.writeName(name)
	case ActionName:
		name := Name(vtype)
		return p.writeName(name)
	case TableName:
		name := Name(vtype)
		return p.writeName(name)
	case ScopeName:
		name := Name(vtype)
		return p.writeName(name)
	case Asset:
		return p.writeAsset(vtype)
	case *NetMessageInfo:
		return p.writeNetMessageInfo(*vtype)
	default:

		if !reflect.ValueOf(v).IsValid() {
			return errors.New("v invalid")
		}
		rv := reflect.Indirect(reflect.ValueOf(v))
		rvt := rv.Type()

		switch rvt.Kind() {

		case reflect.Array:
			l := rvt.Len()

			for i := 0; i < l; i++ {
				if err = p.Pack(rv.Index(i).Interface()); err != nil {
					return
				}
			}
		case reflect.Slice:
			l := rv.Len()
			if err = p.writeUVarInt(l); err != nil {
				return
			}

			for i := 0; i < l; i++ {
				if err = p.Pack(rv.Index(i).Interface()); err != nil {
					return
				}
			}
		case reflect.Struct:
			l := rv.NumField()
			for i := 0; i < l; i++ {
				field := rvt.Field(i)

				tag := field.Tag.Get("eos")
				if tag == "-" {
					continue
				}

				if v := rv.Field(i); rvt.Field(i).Name != "_" {
					if v.CanInterface() {
						isPresent := true
						if tag == "optional" {
							isPresent = !v.IsNil()
							p.writeBool(isPresent)
						}

						if isPresent {
							if err = p.Pack(v.Interface()); err != nil {
								return
							}
						}
					}
				}
			}

		case reflect.Map:
			l := rv.Len()
			if err = p.writeUVarInt(l); err != nil {
				return
			}
			for _, key := range rv.MapKeys() {
				value := rv.MapIndex(key)
				if err = p.Pack(key.Interface()); err != nil {
					return err
				}
				if err = p.Pack(value.Interface()); err != nil {
					return err
				}
			}
		default:
			return errors.New("Pack unsupported type: " + rvt.String())
		}
	}

	return
}

func (p *Packer) write(bytes []byte) (err error) {
	_, err = p.writer.Write(bytes)
	return
}

func (p *Packer) writeByteArray(b []byte) error {
	if err := p.writeUVarInt(len(b)); err != nil {
		return err
	}
	return p.write(b)
}

func (p *Packer) writeUVarInt(v int) (err error) {
	buf := make([]byte, 8)
	l := binary.PutUvarint(buf, uint64(v))
	return p.write(buf[:l])
}

func (p *Packer) writeByte(b byte) (err error) {
	return p.write([]byte{b})
}

func (p *Packer) writeBool(b bool) (err error) {
	var out byte
	if b {
		out = 1
	}
	return p.writeByte(out)
}

func (p *Packer) writeUint16(i uint16) (err error) {
	buf := make([]byte, TypeParseSize.UInt16Size)
	binary.LittleEndian.PutUint16(buf, i)
	return p.write(buf)
}

func (p *Packer) writeInt16(i int16) (err error) {
	return p.writeUint16(uint16(i))
}

func (p *Packer) writeUint32(i uint32) (err error) {
	buf := make([]byte, TypeParseSize.UInt32Size)
	binary.LittleEndian.PutUint32(buf, i)
	return p.write(buf)

}

func (p *Packer) writeUint64(i uint64) (err error) {
	buf := make([]byte, TypeParseSize.UInt64Size)
	binary.LittleEndian.PutUint64(buf, i)
	return p.write(buf)

}

func (p *Packer) writeString(s string) (err error) {
	return p.writeByteArray([]byte(s))
}

func (p *Packer) writeSha256Type(s Sha256Type) error {
	if len(s) == 0 {
		return p.write(bytes.Repeat([]byte{0}, TypeParseSize.Sha256TypeSize))
	}
	return p.write(s)
}

func (p *Packer) writePublicKey(key crypto.PublicKey) (err error) {
	if len(key.Data) != 33 {
		return errors.New("public key length error")
	}

	if err = p.writeByte(byte(key.Prefix)); err != nil {
		return err
	}

	return p.write(key.Data)
}

func (p *Packer) writeSignature(s crypto.Signature) (err error) {
	if len(s.Data) != 65 {
		return errors.New("signature length error")
	}

	if err = p.writeByte(byte(s.Prefix)); err != nil {
		return
	}

	return p.write(s.Data)
}

func (p *Packer) writeName(name Name) error {
	value := StringToName(string(name))
	return p.writeUint64(value)
}

func (p *Packer) writeTstamp(t Tstamp) (err error) {
	n := uint64(t.UnixNano())
	return p.writeUint64(n)
}

func (p *Packer) writeBlockTimestamp(bt BlockTimestamp) (err error) {
	//slot = (sec_since_epoch * 1000 - EpochMs) / IntervalMs;
	n := uint32((uint64(bt.UnixNano())/1000000 - config.BlockTimestampEpoch) / config.BlockIntervalMs)
	return p.writeUint32(n)
}

func (p *Packer) writeAsset(asset Asset) (err error) {

	p.writeUint64(uint64(asset.Amount))
	p.writeByte(asset.Precision)

	symbol := make([]byte, 7, 7)

	copy(symbol[:], []byte(asset.SymbolType.Symbol))
	return p.write(symbol)
}

func (p *Packer) writeTimePoint(time TimePoint) (err error) {
	return p.writeUint64(uint64(time.UnixNano() / 1000))
}

func (p *Packer) writeTimePointSec(time TimePointSec) (err error) {
	return p.writeUint32(uint32(time.Unix()))
}

func (p *Packer) writeNetMessageInfo(messageInfo NetMessageInfo) (err error) {

	if messageInfo.NetMsg != nil {
		buf := new(bytes.Buffer)
		pker := NewPacker(buf)
		err = pker.Pack(messageInfo.NetMsg)
		if err != nil {
			return
		}
		messageInfo.Payload = buf.Bytes()
	}

	messageLen := uint32(len(messageInfo.Payload) + 1)
	err = p.writeUint32(messageLen)
	if err == nil {
		err = p.writeByte(byte(messageInfo.Type))

		if err == nil {
			return p.write(messageInfo.Payload)
		}
	}
	return
}

func MarshalBinary(v interface{}) ([]byte, error) {
	buf := new(bytes.Buffer)
	paker := NewPacker(buf)
	err := paker.Pack(v)
	return buf.Bytes(), err
}
