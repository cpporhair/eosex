package network

import (
	"encoding/binary"
	"fmt"
	"time"

	"errors"
	"reflect"

	"eosex/network/config"
	"eosex/network/crypto"
	"strings"
)

type Unpacker struct {
	data   []byte
	offset int
}

func NewUnpacker(data []byte) *Unpacker {
	return &Unpacker{
		data:   data,
		offset: 0,
	}
}

func (u *Unpacker) remain() int {
	return len(u.data) - u.offset
}

func UnmarshalBinary(data []byte, v interface{}) (err error) {
	unpacker := NewUnpacker(data)
	return unpacker.Unpack(v)
}

func (u *Unpacker) Unpack(v interface{}) (err error) {
	rv := reflect.Indirect(reflect.ValueOf(v))
	if !rv.CanAddr() {
		return errors.New("can only Unpack pointer type")
	}
	rvt := rv.Type()
	if rvt.Kind() == reflect.Ptr {
		rvt = rvt.Elem()
		newRV := reflect.New(rvt)
		rv.Set(newRV)
		rv = reflect.Indirect(newRV)
	}

	switch vtype := v.(type) {
	case *string:
		s, e := u.readString()
		if e != nil {
			err = e
			return
		}
		rv.SetString(s)
		return
	case *int16:
		var n int16
		n, err = u.readInt16()
		rv.SetInt(int64(n))
		return
	case *int32, *IDListModes:
		var n int32
		n, err = u.readInt32()
		rv.SetInt(int64(n))
		return
	case *int64:
		var n int64
		n, err = u.readInt64()
		rv.SetInt(int64(n))
		return
	case *uint16:
		var n uint16
		n, err = u.readUint16()
		rv.SetUint(uint64(n))
		return
	case *uint32:
		var n uint32
		n, err = u.readUint32()
		rv.SetUint(uint64(n))
		return
	case *uint64:
		var n uint64
		n, err = u.readUint64()
		rv.SetUint(n)
		return
	case *Uvarint32:
		var r uint64
		r, err = u.readUvarint()
		rv.SetUint(r)
		return
	case *bool:
		var r bool
		r, err = u.readBool()
		rv.SetBool(r)
		return
	case *BytesType:
		var data []byte
		data, err = u.readByteArray()
		rv.SetBytes(data)
		return
	case *[]byte:
		var data []byte
		data, err = u.readByteArray()
		rv.SetBytes(data)
		return
	case *Name, *AccountName, *PermissionName, *ActionName, *TableName, *ScopeName:
		var n uint64
		n, err = u.readUint64()
		name := NameToString(n)
		rv.SetString(name)
		return
	case *byte, *NetMessageType, *TransactionStatus, *CompressionType, *GoAwayReason:
		var n byte
		n, err = u.readByte()
		rv.SetUint(uint64(n))
		return
	case *Sha256Type, *BlockIdType, *TransactionIdType:
		var s Sha256Type
		s, err = u.readSha256Type()
		rv.SetBytes(s)
		return
	case *crypto.PublicKey:
		var p crypto.PublicKey
		p, err = u.readPublicKey()
		rv.Set(reflect.ValueOf(p))
		return
	case *crypto.Signature:
		var s crypto.Signature
		s, err = u.readSignature()
		rv.Set(reflect.ValueOf(s))
		return
	case *Tstamp:
		var ts Tstamp
		ts, err = u.readTstamp()
		rv.Set(reflect.ValueOf(ts))
		return
	case *BlockTimestamp:
		var bt BlockTimestamp
		bt, err = u.readBlockTimestamp()
		rv.Set(reflect.ValueOf(bt))
		return
	case *TimePoint:
		var jt TimePoint
		jt, err = u.readTimePoint()
		rv.Set(reflect.ValueOf(jt))
		return
	case *TimePointSec:
		var jt TimePointSec
		jt, err = u.readTimePointSec()
		rv.Set(reflect.ValueOf(jt))
		return
	case *Asset:
		var asset Asset
		asset, err = u.readAsset()
		rv.Set(reflect.ValueOf(asset))
		return

	case *TransactionWithID:
		tn, e := u.readByte()
		if e != nil {
			err = fmt.Errorf("TransactionWithID error: %s", e.Error())
			return
		}
		if tn == 0 {
			id, e := u.readSha256Type()
			if e != nil {
				err = fmt.Errorf("readSha256Type error: %s", e.Error())
				return
			}
			trx := TransactionWithID{ID: id}
			rv.Set(reflect.ValueOf(trx))
			return nil
		} else {
			packedTrx := &PackedTransaction{}
			u.Unpack(packedTrx)
			trx := TransactionWithID{Packed: packedTrx}
			rv.Set(reflect.ValueOf(trx))
			return nil
		}
	case **ProducerScheduleType:
		pt, e := u.readByte()
		if e != nil {
			err = fmt.Errorf("ProducerScheduleType erroe: %s", e.Error())
			return
		}
		if pt == 0 {
			*vtype = nil
			return
		}

	case *NetMessageInfo:
		messageInfo, e := u.readNetMessageInfo()
		if e != nil {
			err = fmt.Errorf("NetMessageInfo error, %s", e)
			return
		}
		netMsg := messageInfo.Type.CreateNetMessage()
		if netMsg == nil {
			return errors.New("net message type error")
		}
		unpacker := NewUnpacker(messageInfo.Payload)
		err = unpacker.Unpack(netMsg)
		messageInfo.NetMsg = netMsg

		rv.Set(reflect.ValueOf(*messageInfo))
		return
	case **Action:
		return
	}

	switch rvt.Kind() {
	case reflect.Array:
		for i := 0; i < int(rvt.Len()); i++ {
			if err = u.Unpack(rv.Index(i).Addr().Interface()); err != nil {
				return
			}
		}
		return
	case reflect.Slice:
		var l uint64
		if l, err = u.readUvarint(); err != nil {
			return
		}
		rv.Set(reflect.MakeSlice(rvt, int(l), int(l)))
		for i := 0; i < int(l); i++ {
			if err = u.Unpack(rv.Index(i).Addr().Interface()); err != nil {
				return
			}
		}
	case reflect.Struct:
		err = u.readStruct(v, rvt, rv)
		if err != nil {
			return
		}
	case reflect.Map:
		var l uint64
		if l, err = u.readUvarint(); err != nil {
			return
		}
		kt := rvt.Key()
		vt := rvt.Elem()
		rv.Set(reflect.MakeMap(rvt))
		for i := 0; i < int(l); i++ {
			kv := reflect.Indirect(reflect.New(kt))
			if err = u.Unpack(kv.Addr().Interface()); err != nil {
				return
			}
			vv := reflect.Indirect(reflect.New(vt))
			if err = u.Unpack(vv.Addr().Interface()); err != nil {
				return
			}
			rv.SetMapIndex(kv, vv)
		}

	default:
		return errors.New("unpack unsupported type " + rvt.String())
	}

	return
}

func (u *Unpacker) readBool() (read bool, err error) {

	if u.remain() < TypeParseSize.BoolSize {
		err = errors.New("readBool error")
		return
	}
	b, err := u.readByte()
	if err != nil {
		err = fmt.Errorf("readBool error: %s", err)
		return
	}
	read = b != 0
	return

}

func (u *Unpacker) readByte() (read byte, err error) {

	if u.remain() < TypeParseSize.ByteSize {
		err = errors.New("readByte error")
		return
	}
	read = u.data[u.offset]
	u.offset++
	return
}

func (u *Unpacker) readByteArray() (read []byte, err error) {

	value, err := u.readUvarint()
	if err != nil {
		return nil, err
	}
	if len(u.data) < u.offset+int(value) {
		return nil, errors.New("readByteArray error, type or remain length error")
	}
	read = u.data[u.offset : u.offset+int(value)]
	u.offset += int(value)

	return
}

func (u *Unpacker) readUvarint() (uint64, error) {

	value, n := binary.Uvarint(u.data[u.offset:])
	if n <= 0 {
		return value, errors.New("Uvarint error")
	}

	u.offset += n
	return value, nil
}

func (u *Unpacker) readUint16() (read uint16, err error) {
	if u.remain() < TypeParseSize.UInt16Size {
		err = errors.New("readUint16 error")
		return
	}

	read = binary.LittleEndian.Uint16(u.data[u.offset:])
	u.offset += TypeParseSize.UInt16Size
	return
}

func (u *Unpacker) readInt16() (read int16, err error) {
	n, err := u.readUint16()
	read = int16(n)
	return
}

func (u *Unpacker) readInt32() (read int32, err error) {
	n, err := u.readUint32()
	read = int32(n)
	return
}

func (u *Unpacker) readInt64() (read int64, err error) {
	n, err := u.readUint64()
	read = int64(n)
	return
}

func (u *Unpacker) readUint32() (read uint32, err error) {
	if u.remain() < TypeParseSize.UInt32Size {
		err = errors.New("readUint32 error")
		return
	}

	read = binary.LittleEndian.Uint32(u.data[u.offset:])
	u.offset += TypeParseSize.UInt32Size
	return
}

func (u *Unpacker) readUint64() (read uint64, err error) {
	if u.remain() < TypeParseSize.UInt64Size {
		err = errors.New("readUint64 error")
		return
	}

	data := u.data[u.offset : u.offset+TypeParseSize.UInt64Size]
	read = binary.LittleEndian.Uint64(data)
	u.offset += TypeParseSize.UInt64Size
	return
}

func (u *Unpacker) readString() (read string, err error) {
	data, err := u.readByteArray()
	read = string(data)
	return
}

func (u *Unpacker) readStruct(v interface{}, t reflect.Type, rv reflect.Value) (err error) {
	l := rv.NumField()
	for i := 0; i < l; i++ {
		if tag := t.Field(i).Tag.Get("eos"); tag == "-" {
			continue
		}
		if v := rv.Field(i); v.CanSet() && t.Field(i).Name != "_" {
			iface := v.Addr().Interface()
			if err = u.Unpack(iface); err != nil {
				return
			}
		}
	}
	return
}

func (u *Unpacker) readSha256Type() (read Sha256Type, err error) {

	if u.remain() < TypeParseSize.Sha256TypeSize {
		err = errors.New("readSha256Type error")
		return
	}

	read = Sha256Type(u.data[u.offset : u.offset+TypeParseSize.Sha256TypeSize])
	u.offset += TypeParseSize.Sha256TypeSize
	return
}

func (u *Unpacker) readPublicKey() (read crypto.PublicKey, err error) {

	if u.remain() < TypeParseSize.PublicKeySize {
		err = errors.New("readPublicKey error")
		return
	}
	read = crypto.PublicKey{
		Prefix: crypto.Prefix(u.data[u.offset]),
		Data:   u.data[u.offset+1 : u.offset+TypeParseSize.PublicKeySize], //33 bytes
	}
	u.offset += TypeParseSize.PublicKeySize
	return
}

func (u *Unpacker) readSignature() (read crypto.Signature, err error) {
	if u.remain() < TypeParseSize.SignatureSize {
		err = errors.New("readSignature error")
		return
	}
	read = crypto.Signature{
		Prefix: crypto.Prefix(u.data[u.offset]),
		Data:   u.data[u.offset+1 : u.offset+TypeParseSize.SignatureSize], //65 bytes
	}
	u.offset += TypeParseSize.SignatureSize
	return
}

func (u *Unpacker) readTstamp() (read Tstamp, err error) {

	if u.remain() < TypeParseSize.TstampSize {
		err = errors.New("readTstamp error")
		return
	}

	unixNano, err := u.readUint64()
	read.Time = time.Unix(0, int64(unixNano))
	return
}

func (u *Unpacker) readBlockTimestamp() (read BlockTimestamp, err error) {
	if u.remain() < TypeParseSize.BlockTimestampSize {
		err = errors.New("readBlockTimestamp error")
		return
	}
	n, err := u.readUint32()
	ms := uint64(n)*config.BlockIntervalMs + config.BlockTimestampEpoch
	sec := ms / 1000
	nsec := ms % 1000 * 1000000
	//n -= 8 * 3600
	read.Time = time.Unix(int64(sec), int64(nsec))
	return
}

func (u *Unpacker) readTimePoint() (read TimePoint, err error) {
	n, err := u.readInt64()
	n *= 1000
	read = TimePoint{time.Unix(0, int64(n))}
	return
}

func (u *Unpacker) readTimePointSec() (read TimePointSec, err error) {
	n, err := u.readUint32()
	read = TimePointSec{time.Unix(int64(n), 0).UTC()}
	return
}

func (u *Unpacker) readAsset() (read Asset, err error) {

	amount, err := u.readInt64()
	precision, err := u.readByte()
	if err != nil {
		return read, fmt.Errorf("readAsset error, %s", err)
	}

	data := u.data[u.offset : u.offset+7]
	u.offset += 7

	read = Asset{}
	read.Amount = amount
	read.Precision = precision
	read.SymbolType.Symbol = strings.TrimRight(string(data), "\x00")
	return
}

func (u *Unpacker) readNetMessageInfo() (read *NetMessageInfo, err error) {

	read = &NetMessageInfo{}
	value, err := u.readUint32()
	if err != nil {
		err = fmt.Errorf("net msg length: %s", err)
		return
	}
	read.Length = value
	t, err := u.readByte()
	if err != nil {
		err = fmt.Errorf("net msg type: %s", err)
		return
	}
	read.Type = NetMessageType(t)

	payloadLength := int(value - 1)
	if u.remain() < payloadLength {
		err = fmt.Errorf("net msg payload len: %d, remain: %d", value, u.remain())
		return
	}
	payload := u.data[u.offset : u.offset+payloadLength]
	u.offset += payloadLength

	read.Payload = payload
	return
}
