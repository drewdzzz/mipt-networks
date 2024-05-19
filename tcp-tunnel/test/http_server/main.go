package main

import (
    "fmt"
	"log"
    "net/http"
	"strconv"
	"sync/atomic"
)

var counterValue atomic.Uint64

func hello(w http.ResponseWriter, r *http.Request) {
	log.Println("Received hello req")
    fmt.Fprintf(w, "hello\n")
}

func counter(w http.ResponseWriter, r *http.Request) {
	switch r.Method {
	case "GET":		
		log.Println("Received GET /counter req")
		fmt.Fprintf(w, "Current value = %v\n", counterValue.Load())
	case "POST":
		log.Println("Received POST /counter req")
		/* Call ParseForm() to parse the raw query and update r.PostForm and r.Form. */
		if err := r.ParseForm(); err != nil {
			fmt.Fprintf(w, "ParseForm() err: %v", err)
			return
		}
		inc, err := strconv.Atoi(r.FormValue("add"))
		if err != nil {
			fmt.Fprintf(w, "Cannot parse 'add' argument", err)
			return
		}
		val := counterValue.Add(uint64(inc))
		fmt.Fprintf(w, "New value = %v\n", val)
	default:
		fmt.Fprintf(w, "Sorry, only GET and POST methods are supported.")
	}
}

func main() {
    http.HandleFunc("/hello", hello)
    http.HandleFunc("/counter", counter)
	log.Println("Server is ready to serve")
    log.Fatal(http.ListenAndServe(":3030", nil))
}
