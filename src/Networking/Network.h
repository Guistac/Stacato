#pragma once

#include <asio.hpp>

namespace Network{

	void init();
	void terminate();
	bool isInitialized();
	
	std::unique_ptr<asio::ip::udp::socket> getUdpSocket(int listeningPort, std::vector<int> remoteIp, int remotePort);
	std::unique_ptr<asio::ip::udp::socket> getUdpBroadcastSocket();
}
