from ipaddress import IPv4Address

DNS_RECORD_NUM = 10000

dns_base = 'mipt-1c-dns-server-test'
dns_root = '.drew'

ip_base = 228 * 256 ** 3 + 42 * 256 ** 2

file = open('dns.txt', 'w+')
for i in range(DNS_RECORD_NUM):
    record = f"{dns_base + str(i) + dns_root}\tIN\tA\t{IPv4Address(i) + ip_base}\n"
    file.write(record)
