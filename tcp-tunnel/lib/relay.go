package lib

import (
	"log"
	"net"
)

/* Reads from 'src' and writes to 'dst' */
type Relay struct {
	src *net.TCPConn
	dst *net.TCPConn
	ErrChan chan error
	name string
}

func (r *Relay) Serve() {
	log.Printf("Relay %s started to serve", r.name)
	buf := make([]byte, 4096)
	for {
		nbytes, r_err := r.src.Read(buf)
		if (r_err != nil) {
			log.Printf("Relay %s failed to read: %v", r.name, r_err)
			r.ErrChan <- r_err
			return
		}
		log.Printf("Relay %s successfully read data", r.name)
		_, w_err := r.dst.Write(buf[:nbytes])
		if (r_err != nil) {
			log.Printf("Relay %s failed to write: %v", r.name, w_err)
			r.ErrChan <- r_err
			return
		}
		log.Printf("Relay %s successfully wrote data", r.name)
	}
}

func NewRelay(src *net.TCPConn, dst *net.TCPConn, name string) *Relay {
	return &Relay{
		src: src, dst: dst, ErrChan: make(chan error), name: name,
	}
}
