#pragma once

#include <asio.hpp>

class Network{

public:

	static void init();
	static void terminate();

	static std::unique_ptr<asio::ip::udp::socket> getUdpSocket(int listeningPort, std::vector<int> remoteIp, int remotePort);

private:

	static asio::io_context io_context;
	static std::thread io_context_handler;
};
