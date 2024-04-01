#include "dns.h"
#include "net.hpp"
#include "RemoteResolver.hpp"

#include <iostream>

RemoteResolver::RemoteResolver(uint32_t addr, uint16_t port)
{
	dns_addr_ = addr;
	dns_port_ = port;
	socket_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_ < 0)
		throw std::runtime_error("RemoteResolver: cannot setup socket");
}

int64_t
RemoteResolver::resolve(const std::string &domain, const dns_question &_q_arg)
{
	char answer_buffer[MAX_ANS_SIZE] = {};
	char query_buffer[MAX_QUERY_SIZE] = {};

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(dns_port_)
	};
	addr.sin_addr.s_addr = htonl(dns_addr_);

	struct dns_header dns_head = {};
	dns_head.ID = htons(0x2929);
	dns_head.Opcode = 0;
	dns_head.QR = 0;
	dns_head.question_num = htons(1);
	dns_head.RD = 1;

	struct dns_question q = {.class_ = htons(1), .type = htons(1)};

	int domain_len = domain.size();

	char *buf = query_buffer;
	memcpy(buf, &dns_head, sizeof(dns_head));
	buf += sizeof(dns_head);

	memcpy(buf + 1, domain.c_str(), domain_len + 1);
	name_to_query(buf);
	buf += domain_len + 2;

	memcpy(buf, &q, sizeof(q));
	buf += sizeof(q);

	int rc = sendto(socket_, query_buffer, buf - query_buffer, 0,
			(const sockaddr *)&addr, sizeof(addr));
	if (rc < 0)
		return -1;

	rc = recvfrom(socket_, answer_buffer, MAX_ANS_SIZE, 0, NULL, NULL);
	if (rc < 0)
		return -1;
	
	struct dns_header *ans_head = (dns_header*)answer_buffer;
	if (htons(ans_head->question_num) != 1) {
		std::cerr << "RemoteResolver - unexpected question num: "
			  << htons(ans_head->question_num) << std::endl;
		return -1;
	}
	char *answer_ptr = answer_buffer + sizeof(dns_header) + domain_len + 2 + sizeof(dns_question);

	/* Pointer is used instead of domain if compression is used. */
	if ((*answer_ptr & 192) == 192)
		answer_ptr += 2;
	else
		answer_ptr += domain_len + 2;
	
	struct dns_answer *answer_info = (dns_answer *)answer_ptr;

	bool found = false;
	for (int i = 0; i < ntohs(ans_head->ans_num) && !found; ++i) {
		if (ntohs(answer_info->type) == TYPE_A_ID) {
			found = true;
			break;
		}
		answer_ptr += sizeof(answer_info) + ntohs(answer_info->len);
		/* Pointer is used instead of domain if compression is used. */
		if ((*answer_ptr & 192) == 192)
			answer_ptr += 2;
		else
			answer_ptr += domain_len + 2;
	}

	if (!found)
		return -1;
	
	char *ip_pos = (char *)answer_info + sizeof(dns_answer);
	return ntohl(*(uint32_t *)ip_pos);
}
