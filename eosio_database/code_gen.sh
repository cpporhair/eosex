#/bin/sh
protoc -I=./proto --cpp_out=./src/protocol/ ./proto/message.proto
