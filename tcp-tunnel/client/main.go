package main

import (
	"flag"
	"io"
	"log"
	"net"

	"tcp-tunnel/lib"
)

func main() {
	resourceAddrStr := flag.String("resource", "127.0.0.1:3030", "Address of resource (`ip`:`port`)")
	serverAddrStr := flag.String("server", "127.0.0.1:3031", "Address of server (`ip`:`port`)")
	flag.Parse()

	/* Establish TCP connection to the server. */
	serverAddr, err := net.ResolveTCPAddr("tcp", *serverAddrStr)
    if err != nil {
		panic(err)
    }
	serverConn, err := net.DialTCP("tcp", nil, serverAddr)
	if err != nil {
		panic(err)
	}

	/* Establish TCP connection to the resource. */
    resourceAddr, err := net.ResolveTCPAddr("tcp", *resourceAddrStr)
	if err != nil {
		panic(err)
	}
	resourceConn, err := net.DialTCP("tcp", nil, resourceAddr)
	if err != nil {
		panic(err)
	}

	/* Create relays for both directions. */
	fromServer := lib.NewRelay(serverConn, resourceConn, "tunnelServer -> resource")
	toServer := lib.NewRelay(resourceConn, serverConn, "resource -> tunnelServer")

	/* Start the relays. */
	go fromServer.Serve()
	go toServer.Serve()

	/* Catch errors from the relays. */
	select {
	case serverErr := <- fromServer.ErrChan:
		if serverErr == io.EOF {
			log.Println("Server disconnected, shutting down")
			return
		}
		panic(serverErr)
	case resourceErr := <- toServer.ErrChan:
		if resourceErr == io.EOF {
			log.Println("Resource disconnected, shutting down")
			return
		}
		panic(resourceErr)
	}
}
