#pragma once

#include <string>

#include "dns.h"
#include "LocalResolver.hpp"
#include "RemoteResolver.hpp"

class Resolver {
public:
	Resolver(std::ifstream &stream, uint32_t addr, uint16_t port):
		local_resolver_(stream), remote_resolver_(addr, port) {}

	int64_t
	resolve(const std::string &domain, const dns_question &q)
	{
		int64_t addr = local_resolver_.resolve(domain, q);
		/* Fallback to remote resolver if the local fails. */
		if (addr < 0)
			addr = remote_resolver_.resolve(domain, q);
		return addr;
	}
private:
	LocalResolver local_resolver_;
	RemoteResolver remote_resolver_;
};
