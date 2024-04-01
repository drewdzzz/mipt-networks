# DNS-RESOLVER

## Nota bene

The resolver accepts **TCP** conections! Do not forget to add `+tcp` option when using `dig`.

## Description

A simple DNS resolver, supports only A-records of class Internet. Listens on `localhost:8080`. Supports only one DNS question per request.

The resolver has its own storage of DNS records (it is read from file on start).
When requested domain is not found in local storage, it fallbacks to Google DNS server.

## How to run

```sh
mkdir build && cd build
cmake ..
make -j
./src/DnsResolver [/path/to/file/with/records]
```

If path to storage is not passed, the resolver tries to open `./dns.txt` file.

## How to test

In the first terminal:
```sh
mkdir build && cd build
cmake ..
make -j

# Generate 10k DNS records
python3 ../tools/gen_dns.py

# Start the resolver
./src/DnsResolver
```

In another terminal:
```sh
cd tools
python3 gen_bench.py

# You may shuffle "bench.txt" here if you want to.

source bench.sh
```

The `bench.sh` makes 1000 requests and measures elapsed time with Linux `time` utility.

If you want to make sure if all IP addresses are OK, you can run `source bench_describe.sh` - it does not measure elapsed time but shows all the resolved addresses.
