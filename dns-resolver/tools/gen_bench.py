
# Benchmark spec:
# 1 - 25% similar requests from file
# 2 - 10% non-existent domain requests
# 3 - 50% different requests from file
# 4 - 15% similar existent on remote server domain requests

BENCH_SIZE = 1000

dns_base = 'mipt-1c-dns-server-test'
dns_root = '.drew'

file = open('bench.txt', 'w+')

# Step 1
STEP_SIZE = int(BENCH_SIZE * 0.25)
domain = dns_base + str(42) + dns_root
for i in range(STEP_SIZE):
    file.write(f"{domain}\n")

# Step 2
STEP_SIZE = int(BENCH_SIZE * 0.1)
domain = 'this-domain-does-not-exist.test.non-existent'
for i in range(STEP_SIZE):
    file.write(f"{domain}\n")

# Step 4
STEP_SIZE = int(BENCH_SIZE * 0.5)
for i in range(STEP_SIZE):
    domain = dns_base + str(i) + dns_root
    file.write(f"{domain}\n")

# Step 4
STEP_SIZE = int(BENCH_SIZE * 0.15)
domain = 'google.com'
for i in range(STEP_SIZE):
    file.write(f"{domain}\n")
