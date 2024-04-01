#pragma once

#include <string>
#include <cstdint>

struct RemoteResolver {
	RemoteResolver(uint32_t addr, uint16_t port);

	int64_t
	resolve(const std::string &domain, const dns_question &q);
private:
	int socket_;
	int dns_port_;
	int dns_addr_;
};
