#include <pch.h>
#include "OscSocket.h"

#include "Network.h"

OscSocket::OscSocket(size_t bufferSize) : inBuffer(bufferSize) {}

OscSocket::~OscSocket() {
	close();
}

void OscSocket::open(int receivingPort, std::vector<int> destinationIp, int destinationPort) {
	socket = Network::getUdpSocket(receivingPort, destinationIp, destinationPort);
	if (socket == nullptr) return;
	while (!socket->is_open()) {}
	b_isOpen = true;
	asyncReceive();
	Logger::debug("Started OSC Socket, listening on port {}, talking to remote address {}.{}.{}.{} on port {}",
		receivingPort, destinationIp[0], destinationIp[1], destinationIp[2], destinationIp[3], destinationPort);
}

void OscSocket::close() {
	if (isOpen()) {
		try {
			socket->close();
			socket.release();
			b_isOpen = false;
			mutex.lock();
			inPackets.clear();
			mutex.unlock();
			Logger::info("Closing OSC Socket");
		}
		catch (std::exception e) {
			Logger::info("Failed To Close OSC socket: {}", e.what());
		}
	}
}

bool OscSocket::isOpen() {
	return b_isOpen;
}

bool OscSocket::messageAvailable() {
	bool empty;
	mutex.lock();
	empty = inPackets.empty();
	mutex.unlock();
	return !empty;
}

std::vector<std::shared_ptr<OscMessage>> OscSocket::getMessages() {
	std::vector<std::shared_ptr<OscMessage>> output;
	mutex.lock();
	if (!inPackets.empty()) {
		inPackets.swap(output);
	}
	mutex.unlock();
	return output;
}

void OscSocket::send(std::shared_ptr<OscMessage> message) {
	if (!isOpen()) return Logger::warn("Could not send osc message {}, socket is not open", message->getAddress());
	char outBuffer[1024];
	int size = 1024;
	int bufferSize = message->getBuffer(outBuffer, size);
	try {
		socket->async_send(asio::buffer(outBuffer, bufferSize), [](asio::error_code error, size_t byteCount) {
			if (error) Logger::debug("Failed to send OSC Message: {}", error.message());
		});
	}
	catch (std::exception e) {
		Logger::error("Failed to start async_send: {}", e.what());
	}
}

void OscSocket::send(char* buffer, int size) {
	if (!isOpen()) return;
	try {
		socket->async_send(asio::buffer(buffer, size), [](asio::error_code error, size_t byteCount) {
			if (error) Logger::debug("Failed to send OSC Message: {}", error.message());
			});
	}
	catch (std::exception e) {
		Logger::error("Failed to start async_send: {}", e.what());
	}
}

void OscSocket::asyncReceive() {
	try {
		auto buffer = asio::buffer(inBuffer.data(), inBuffer.size());
		socket->async_receive(buffer, [this](asio::error_code error, size_t byteCount) {
			if (error) {
				Logger::debug("Failed to receive OSC Message: {}", error.message());
			}else {
				std::vector<std::shared_ptr<OscMessage>> messages = OscPacket::getOscMessages(inBuffer.data(), byteCount);
				mutex.lock();
				for (auto message : messages) {
					inPackets.push_back(message);
				}
				mutex.unlock();
				asyncReceive();
			}
		});
	}
	catch (std::exception e) {
		Logger::error("Failed to start async_receive OSC Message: {}", e.what());
	}
}
