#pragma once
#include <arpa/inet.h>
#include <exception>
#include <fstream>
#include <map>
#include <string>
#include <iostream>

class dns_storage {
public:
	dns_storage(std::ifstream &stream)
	{
		std::string domain, class_, type, ip_str;
		while (stream >> domain >> class_ >> type >> ip_str) {
			if (class_ != "IN" || type != "A")
				throw std::runtime_error(
					"dns_storage: only internet A-records "
					"are supported");
			in_addr addr;
			int rc = inet_aton(ip_str.c_str(), &addr);
			if (rc != 1)
				throw std::runtime_error("dns_storage: invalid ip addr");
			/*
			 * Do not forget to change byte order of IP:
			 * result of inet_aton is in network byte order.
			 */
			data_.emplace(std::make_pair(std::move(domain), ntohl(addr.s_addr)));
		}
	}

	int64_t
	get_ip(const std::string &domain)
	{
		if (auto search = data_.find(domain); search != data_.end())
			return search->second;
		else
			return -1;
	}
private:
	std::map<std::string, int64_t> data_;
};
