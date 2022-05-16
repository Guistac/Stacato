#include <pch.h>

#include "Network.h"

namespace Network{

asio::io_context io_context;
std::thread io_context_handler;
bool b_initialized;

void init() {
	io_context_handler = std::thread([&]() {
		pthread_setname_np("Asio Network Thread");
		asio::io_context::work dummyWork(io_context);
		b_initialized = true;
		Logger::info("===== Started IP Network IO Context");
		io_context.run();
		b_initialized = false;
		Logger::info("===== Stopped IP Network IO Context");
	});
}

void terminate() {
	io_context.stop();
	if (io_context_handler.joinable()) io_context_handler.join();
}

std::unique_ptr<asio::ip::udp::socket> getUdpSocket(int listeningPort, std::vector<int> remoteIp, int remotePort) {
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
		Logger::error("UDP Socket Creation Network Error: {}", e.what());
	}
	return socket;
}

bool isInitialized(){
	return b_initialized;
}

}
