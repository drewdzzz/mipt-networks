package main

import (
    "fmt"
	"flag"
	"io"
    "log"
	"net"
	"sync"

	"tcp-tunnel/lib"
)

func main() {
	clientPort := flag.Int("clientport", 3031, "Port to which client will be connected")
	userPort := flag.Int("userport", 3032, "Port to which user will be connected")
	flag.Parse()

	/*
	 * Simultaneously listen on two ports (one for client, another for user).
	 * WaitGroup is used to wait while both connections are established.
	 */
	var wg sync.WaitGroup
	wg.Add(2)

	var clientConn *net.TCPConn
	var userConn *net.TCPConn

	go func() {
		addr, err := net.ResolveTCPAddr("tcp", fmt.Sprintf(":%d", *clientPort))
		if err != nil {
			panic(err)
		}
		lis, err := net.ListenTCP("tcp", addr)
    	if err != nil {
        	panic(fmt.Errorf("can't bind socket: %v", err))
    	}
		log.Printf("Listening for client on %v\n", lis.Addr())
		defer lis.Close()

		clientConn, err = lis.AcceptTCP()
		if err != nil {
			panic(err)
		}
		log.Println("Accepted connection from client")
		wg.Done()
	} ()

	go func() {
		addr, err := net.ResolveTCPAddr("tcp", fmt.Sprintf(":%d", *userPort))
		if err != nil {
			panic(err)
		}
		lis, err := net.ListenTCP("tcp", addr)
    	if err != nil {
        	panic(fmt.Errorf("can't bind socket: %v", err))
    	}
		log.Printf("Listening for user on %v\n", lis.Addr())
		defer lis.Close()

		userConn, err = lis.AcceptTCP()
		if err != nil {
			panic(err)
		}
		log.Println("Accepted connection from user")
		wg.Done()
	} ()

	wg.Wait()

	/* Once the connections are established, create relays for both directions. */
	fromUser := lib.NewRelay(userConn, clientConn, "user -> tunnelClient")
	toUser := lib.NewRelay(clientConn, userConn, "tunnelClient -> user")

	/* Start the relays. */
	go fromUser.Serve()
	go toUser.Serve()

	/* Catch errors from the relays. */
	select {
	case userErr := <- fromUser.ErrChan:
		if userErr == io.EOF {
			log.Println("User disconnected, shutting down")
			return
		}
		panic(userErr)
	case clientErr :=  <- toUser.ErrChan:
		if clientErr == io.EOF {
			log.Println("Tunnel client disconnected, shutting down")
			return
		}
		panic(clientErr)
	}
}
