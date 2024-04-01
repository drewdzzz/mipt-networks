#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <vector>

typedef unsigned char uc;

#if __has_attribute(packed) || defined(__GNUC__)
#  define PACKED  __attribute__((packed))
#elif defined(__CC_ARM)
#  define PACKED __packed
#else
#error PACKED is not supported
#endif

struct PACKED dns_header
{
	uint16_t ID;

	uc RD : 1;
	uc TC : 1;
	uc AA : 1;
	uc Opcode : 4;
	uc QR : 1;

	uc Rcode : 4;
	uc CD : 1;
	uc AD : 1;
	uc Z : 1;
	uc RA : 1;

	uint16_t question_num;
	uint16_t ans_num;
	uint16_t upoln_num;
	uint16_t add_num;
};

struct PACKED dns_answer
{
	uint16_t type;
	uint16_t class_;
	uint32_t TTL;
	uint16_t len;
};

struct PACKED dns_question
{
	uint16_t type;
	uint16_t class_;
};

/* Is used to transfer data within the server. */
struct dns_packet {
	dns_header h;
	std::vector<std::string> domains;
	std::vector<dns_question> qs;
	std::vector<int64_t> ips;
	std::vector<dns_answer> as;
	/* Authorities, aditional sections and so on. */
	std::string tail;
};

enum
{
	/* DNS protocol data. */
	DOMAIN_MAX_SIZE = 300,
	MAX_ANS_SIZE = 1024,
	MAX_QUERY_SIZE = 1024,
	COMP_NAME_SIZE = 2,
	TYPE_A_ID = 1,
	CLASS_IN_ID = 1,

	/* Fallback DNS resolver. */
	FALLBACK_DNS = 0x08080808,
	FALLBACK_DNS_PORT = 53,

};
