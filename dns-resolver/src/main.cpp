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

#include "Resolver.hpp"
#include "net.hpp"


int
main(int argc, const char **argv)
{
	std::string filename("dns.txt");
	uint16_t port = 8080;
	if (argc >= 2)
		filename = std::string(argv[1]);
	if (argc >= 3)
		port = atol(argv[2]);
	
	std::ifstream stream(filename);
	if (!stream.good()) {
		std::cout << "Can't open file with DNS records" << std::endl;
		return 1;
	}
	Resolver resolver(stream, FALLBACK_DNS, FALLBACK_DNS_PORT);
	stream.close();

	/* Listen on particular port. */
	struct in_addr addr;
	addr.s_addr = 0;
	struct sockaddr_in ip_sock = {};
	ip_sock.sin_family = AF_INET;
	ip_sock.sin_port = htons(port);
	ip_sock.sin_addr = addr;
	struct sockaddr* sock_ptr = (struct sockaddr*)(&ip_sock);
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	bind(sock_fd, sock_ptr, sizeof(ip_sock));
	listen(sock_fd, SOMAXCONN);

	std::cout << "Listening on port " << ntohs(ip_sock.sin_port) << std::endl;
	std::cout << "The server is ready to accept requests" << std::endl;

	while (true) {
		int client_fd = accept(sock_fd, 0, 0);
		if (client_fd < 0) {
			std::cout << "Can't accept conection, errno: "
				  << strerror(errno) << std::endl;
			return 1;
		}
		std::cout << "Accepted connection" << std::endl;

		auto packet = read_request(client_fd);

		int64_t ip = -1;
		if (packet.has_value()) {
			// TODO: several domains
			ip = resolver.resolve(packet->domains[0], packet->qs[0]);
			if (ip < 0) {
				std::cout << "Failed to resolve ip" << std::endl;
				std::cout << "Domain is " << packet->domains[0] << std::endl;
			} else {
				std::cout << ip << std::endl;
			}
			packet->ips.push_back(ip);
			if (!write_answer(client_fd, packet.value()))
				std::cerr << "Cannot write answer" << std::endl;
			else
				std::cout << "Response written succesfully" << std::endl;
		} else {
			std::cout << "Failed to parse request" << std::endl;
		}

		close(client_fd);
		shutdown(client_fd, SHUT_RDWR);
	}

	close(sock_fd);
	shutdown(sock_fd, SHUT_RDWR);
}
