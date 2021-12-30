#pragma once

#include <asio.hpp>
#include <tinyosc.h>
#include <OscMessage.h>

class OscSocket {

public:

	OscSocket(size_t bufferSize);
	~OscSocket();

	void open(int listeningPort, std::vector<int>, int remotePort);
	void close();
	bool isOpen();

	void send(std::shared_ptr<OscMessage> message);
	void send(char* buffer, int size);
	bool messageAvailable();
	std::vector<std::shared_ptr<OscMessage>> getMessages();

private:

	bool b_isOpen = false;
	std::unique_ptr<asio::ip::udp::socket> socket;

	void asyncReceive();
	std::vector<char> inBuffer;
	std::vector<std::shared_ptr<OscMessage>> inPackets;
	std::mutex mutex;
};
