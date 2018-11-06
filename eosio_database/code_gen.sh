#/bin/sh
protoc -I=./proto --cpp_out=./src/protocol/ ./proto/fork_database_service.proto
