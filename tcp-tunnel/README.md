# TCP tunnel

A simple single-use TCP tunnel. After connection is closed, the server is shut down due to EOF.

## Launch

From `tcp-tunnel` directory:
* Launch server - `go run server/main.go`
* Launch client - `go run client/main.go`

By default, server listens for client on 3031 port and for user on 3032 port, client connects to resource at "127.0.0.1:3030" and to server at "127.0.0.1:3031". It can be changed with options `clientport` and `userport` for server and with `resource` and `server` for client.

## Quick local test

One can launch a simple http server with `go run test/http_server/main.go`.

Also, one can setup the server with TCP tunnel:

```sh
cd test
./launch.sh
```

**Make sure** that everything is OK (it takes several seconds to set up) and there are no errors like "cannot bind" in logs.

After that, one can send a bunch of requests:

```sh
./request.sh
```
