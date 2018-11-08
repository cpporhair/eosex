//
// Created by 杨文宇 on 2018/11/8.
//
#pragma once

#include <google/protobuf/message.h>

class serializer {
public:
    void serialize( ::google::protobuf::Message &msg ){
        _length = sizeof(int32_t) + msg.ByteSize();
        int32_t len = msg.ByteSize();
        std::memcpy(_buf,&len,sizeof(int32_t));
        msg.SerializeToArray(_buf+sizeof(int32_t),len);
    }

    const char* data() const {
        return _buf;
    }

    const int32_t length() const {
        return _length;
    }

private:
    char    _buf[1024*1024]{};
    int32_t _length;
};