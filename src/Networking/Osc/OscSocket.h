#pragma once

#include <asio.hpp>
#include <tinyosc.h>
#include <OscMessage.h>

class OscSocket {
public:

	OscSocket(size_t bufferSize);
	~OscSocket();

	void open(int listeningPort, std::vector<int>, int remotePort);
	void openBroadcast(uint32_t network, uint32_t mask, uint16_t port);
	void close();
	bool isOpen();

	void send(std::shared_ptr<OscMessage> message);
	
	bool messageAvailable();
	std::vector<std::shared_ptr<OscMessage>> getMessages();

private:

	bool b_isOpen = false;
	bool b_broadcast = false;
	std::unique_ptr<asio::ip::udp::socket> udpSocket;

	asio::ip::udp::endpoint broadcastEndpoint;

	void asyncReceive();
	std::vector<char> inBuffer;
	std::vector<std::shared_ptr<OscMessage>> inPackets;
	std::mutex mutex;
};
