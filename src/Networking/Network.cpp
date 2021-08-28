#include <pch.h>

#include "Network.h"

asio::io_context Network::io_context;
std::thread Network::io_context_handler;

void Network::init() {
	io_context_handler = std::thread([&]() {
		asio::io_context::work dummyWork(io_context);
		std::cout << "start network io context" << std::endl;
		io_context.run();
		std::cout << "exit network io context" << std::endl;
	});
}

void Network::terminate() {
	io_context.stop();
	if (io_context_handler.joinable()) io_context_handler.join();
}

std::unique_ptr<asio::ip::udp::socket> Network::getUdpSocket(int listeningPort, std::vector<int> remoteIp, int remotePort) {
	std::unique_ptr<asio::ip::udp::socket> socket = nullptr;
	if (remoteIp.size() != 4) return socket;
	for (int octet : remoteIp) if (octet > 255 || octet < 0) return socket;
	char ip[32];
	sprintf(ip, "%i.%i.%i.%i", remoteIp[0], remoteIp[1], remoteIp[2], remoteIp[3]);

	try {
		asio::ip::address_v4 remoteIp = asio::ip::make_address_v4(ip);
		asio::ip::udp::endpoint localEndpoint(asio::ip::udp::v4(), listeningPort);
		asio::ip::udp::endpoint remoteEndpoint(remoteIp, remotePort);
		socket = std::make_unique<asio::ip::udp::socket>(io_context, localEndpoint);
		socket->async_connect(remoteEndpoint, [](asio::error_code) {});
	}
	catch (std::exception e) {
		std::cerr << e.what() << std::endl;
	}
	return socket;
}
