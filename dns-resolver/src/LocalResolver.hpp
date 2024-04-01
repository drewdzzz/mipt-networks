#pragma once

#include <string>
#include <cstdint>

#include "dns.h"
#include "dns_storage.hpp"

struct LocalResolver {
	LocalResolver(std::ifstream &stream) : storage_(stream) {}

	int64_t
	resolve(const std::string &domain, const dns_question &q)
	{
		if (ntohs(q.type) != 1 || ntohs(q.class_) != 1)
			return -1;
		return storage_.get_ip(domain);
	}
private:
	dns_storage storage_;
};
