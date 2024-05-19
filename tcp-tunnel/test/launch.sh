#!/bin/sh

cd ..
go run server/main.go &
sleep 2
go run test/http_server/main.go &
sleep 2
go run client/main.go
