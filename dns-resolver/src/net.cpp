#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>

#include <iostream>
#include <optional>

#include "dns.h"

enum {
	BUF_SIZE = 1024 * 10,
};

static char BUF[BUF_SIZE];

int
readn(int fd, char *buf, size_t *offset, size_t required_offset, size_t buf_size) {
	while (*offset < required_offset) {
		ssize_t rc = read(fd, buf + *offset, buf_size - *offset);
		if (rc <= 0)
			return -1;
		*offset += rc;
	}
	return 0;
}

#if 0
/** Prints buffer in hex - is used for debug. */
static void
print_bytes(std::ostream& out, const char *title, const unsigned char *data, size_t dataLen, bool format = true) {
    out << title << std::endl;
    out << std::setfill('0');
    for(size_t i = 0; i < dataLen; ++i) {
        out << std::hex << std::setw(2) << (int)data[i];
        if (format) {
            out << (((i + 1) % 16 == 0) ? "\n" : " ");
        }
    }
    out << std::endl;
}
#endif

std::optional<dns_packet>
read_request(int fd)
{
	size_t offset = 0;
	char *buf = BUF;

	/* Read message length. */
	if (readn(fd, buf, &offset, 2, BUF_SIZE))
		return std::nullopt;
	
	size_t len = ntohs(*(uint16_t *)buf);

	/* Read message. */
	if (readn(fd, buf, &offset, 2 + len, BUF_SIZE))
		return std::nullopt;

	/* Set cursor to the message start and save it. */
	buf = BUF + 2;
	const char *const msg_start = buf;

	/* Parse DNS header */
	dns_header *header = (dns_header *)(buf);
	if (ntohs(header->question_num) != 1 || ntohs(header->ans_num) != 0) {
		std::cout << "Unexpected question and ans num: "
			  << ntohs(header->question_num) << " and "
			  << ntohs(header->ans_num) << std::endl;
		return std::nullopt;
	}

	/* Advance cursor. */
	buf += sizeof(dns_header);

	/* Read domain name */
	std::string domain;
	domain.reserve(DOMAIN_MAX_SIZE);
	uint8_t label_size = 0;
	do {
		/* Add delimiter if it's not first iteration. */
		if (label_size != 0)
			domain += std::string(".");

		label_size = *buf;

		/* Advance cursor. */
		buf++;

		/* If label is empty, it's the last one. */
		if (label_size == 0)
			break;

		/* Actually read the label. */
		domain += std::string(buf, label_size);

		/* Advance cursor. */
		buf += label_size;
	} while (label_size != 0);

	if (domain.empty())
		return std::nullopt;
	/* Pop trailing delimiter. */
	domain.pop_back();

	dns_question *q = (dns_question *)buf;

	/* Only A-records with class IN are supported. */
	if (ntohs(q->class_) != 1 && ntohs(q->type) != 1) {
		std::cerr << "Only A-records with class IN are suppoted" << std::endl;
		return std::nullopt;
	}

	/* Advance cursor. */
	buf += sizeof(*q);

	dns_packet packet = {
		.h = *header,
		.domains = {domain},
		.qs = {*q},
		.ips = {},
		.as = {},
		.tail = std::string(buf, len - (buf - msg_start)),
	};

	return packet;
}

void
name_to_query(char *name)
{
	char *len_ptr = name;
	*name = 0;
	name++;

	while (1)
	{
		while (*name != '\0' && *name != '.')
		{
			name++;
			++(*len_ptr);
		}

		if (*name == '.')
		{
			len_ptr = name;
			*len_ptr = 0;
			++name;
		}
		else
			break;
	}
}

bool
write_answer(int fd, dns_packet &p)
{
	char *buf = BUF + 2;
	/* Fill header. */
	dns_header *h = (dns_header *)buf;
	*h = p.h;
	h->QR = 1;
	h->AA = 0;
	h->TC = 0;
	h->RA = 0;
	h->Rcode = p.ips[0] > 0 ? 0 : 3;
	h->question_num = htons(1);
	h->ans_num = htons(p.ips[0] > 0 ? 1 : 0);
	/* Authorities and additional info are copied from original header. */

	uint16_t len = sizeof(*h);

	/* Fill question. */
	std::string &domain = p.domains[0];
	char *qname = buf + len;
	memcpy(qname + 1, domain.c_str(), domain.size() + 1);
	name_to_query(qname);

	len += domain.size() + 2;

	dns_question *q = (dns_question *)(buf + len);
	*q = p.qs[0];

	len += sizeof(*q);

	/* Fill answer. */
	if (p.ips[0] > 0) {
		char *name = buf + len;
		memcpy(name + 1, domain.c_str(), domain.size() + 1);
		name_to_query(name);

		len += domain.size() + 2;
	
		dns_answer *a = (dns_answer *)(buf + len);
		a->type = htons(1);
		a->class_ = htons(1);
		a->TTL = htonl(0);
		a->len = htons(4);
	
		len += sizeof(*a);
	
		uint32_t ip_addr = p.ips[0];
		uint32_t *addr = (uint32_t *)(buf + len);
		*addr = htonl(ip_addr);
		len += 4;
	}

	if (p.tail.size() > 0) {
		memcpy(buf + len, p.tail.c_str(), p.tail.size());
		len += p.tail.size();
	}

	uint16_t *packet_len = (uint16_t *)BUF;
	*packet_len = htons(len);

	/** Extra bytes for TCP. */
	len += 2;

	/* Write to the client. */
	size_t pos = 0;
	while (pos < len) {
		ssize_t rc = write(fd, BUF + pos, len - pos);
		if (rc <= 0)
			return false;
		pos += rc;	
	}
	return true;
}
