#!/bin/sh
curl localhost:3032/hello localhost:3032/counter --next -d "add=1" localhost:3032/counter --next -d "add=41" localhost:3032/counter --next localhost:3032/counter
