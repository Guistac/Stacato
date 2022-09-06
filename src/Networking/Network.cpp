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
		return nullptr;
	}
	return socket;
}

std::unique_ptr<asio::ip::udp::socket> getUdpBroadcastSocket(std::vector<uint8_t> networkIp, std::vector<uint8_t> networkMask, uint16_t remotePort){
	std::unique_ptr<asio::ip::udp::socket> socket = std::make_unique<asio::ip::udp::socket>(io_context);
	uint32_t address_u32 = networkIp[0] << 24 | networkIp[1] << 16 | networkIp[2] << 8 | networkIp[3];
	uint32_t mask_u32 = networkMask[0] << 24 | networkMask[1] << 16 | networkMask[2] << 8 | networkMask[3];
	
	try{
	
		asio::ip::address_v4 address = asio::ip::make_address_v4(address_u32);
		asio::ip::address_v4 mask = asio::ip::make_address_v4(mask_u32);
		asio::ip::network_v4 network(address, mask);
		asio::ip::address_v4 broadcastAddress = network.broadcast();
		asio::ip::udp::endpoint broadcastEndpoint(broadcastAddress, remotePort);
		
		socket->async_connect(broadcastEndpoint, [](asio::error_code error) {
			Logger::warn("Could not connect to Udp Broadcast endpoint : {}", error.message());
		});
		
		if(socket == nullptr) return nullptr;
		asio::error_code error;
		socket->open(asio::ip::udp::v4(), error);
		if(error) return nullptr;
		socket->set_option(asio::ip::udp::socket::reuse_address(true));
		socket->set_option(asio::socket_base::broadcast(true));

	}catch(std::exception e){
		Logger::warn("Coult not create udp broadcast socket : {}", e.what());
		return nullptr;
	}
	
	
	return socket;
}

bool isInitialized(){
	return b_initialized;
}

}
