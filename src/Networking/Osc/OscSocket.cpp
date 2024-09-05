#include <pch.h>
#include "OscSocket.h"

#include "Networking/Network.h"

OscSocket::OscSocket(size_t bufferSize) : inBuffer(bufferSize) {}

OscSocket::~OscSocket() { close(); }

void OscSocket::open(int receivingPort, std::vector<int> destinationIp, int destinationPort) {
	b_isOpen = false;
	udpSocket = Network::getUdpSocket(receivingPort, destinationIp, destinationPort);
	if (udpSocket == nullptr) return;
	while (!udpSocket->is_open()) {}
	b_isOpen = true;
	asyncReceive();
}

void OscSocket::openBroadcast(uint32_t network_u32, uint32_t mask_u32, uint16_t port){
	b_isOpen = false;
	b_broadcast = true;
	udpSocket = Network::getUdpBroadcastSocket();
	b_isOpen = udpSocket != nullptr;

	asio::ip::address_v4 address = asio::ip::make_address_v4(network_u32);
	asio::ip::address_v4 mask = asio::ip::make_address_v4(mask_u32);
	asio::ip::network_v4 network(address, mask);
	broadcastEndpoint = asio::ip::udp::endpoint(network.broadcast(), port);
}

void OscSocket::close() {
	if (isOpen()) {
		try {
			udpSocket->close();
			udpSocket.release();
			b_isOpen = false;
			mutex.lock();
			inPackets.clear();
			mutex.unlock();
		}
		catch (std::exception e) {
			Logger::error("Failed To Close OSC socket: {}", e.what());
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
	if (!isOpen()) {
		Logger::warn("Could not send osc message {}, socket is not open", message->getAddress());
		return;
	}
	char outBuffer[1024];
	int size = 1024;
	int bufferSize = message->getBuffer(outBuffer, size);
	try {
		if(b_broadcast){
			udpSocket->async_send_to(asio::buffer(outBuffer, bufferSize), broadcastEndpoint, [this](asio::error_code error, size_t byteCount){
				if (error) Logger::debug("Failed to send broadcast OSC Message: {}", error.message());
			});
		}
		else{
			udpSocket->async_send(asio::buffer(outBuffer, bufferSize), [](asio::error_code error, size_t byteCount) {
				if (error) Logger::debug("Failed to send OSC Message: {}", error.message());
			});
		}
	}
	catch (std::exception e) {
		Logger::error("Failed to start async_send: {}", e.what());
	}
}


void OscSocket::asyncReceive() {
	try {
		auto buffer = asio::buffer(inBuffer.data(), inBuffer.size());
		udpSocket->async_receive(buffer, [this](asio::error_code error, size_t byteCount) {
			if (error) {
				Logger::debug("Failed to receive OSC Message: {}", error.message());
			}else {
				std::vector<std::shared_ptr<OscMessage>> messages = OscPacket::getOscMessages(inBuffer.data(), byteCount);
				mutex.lock();
				for (auto message : messages) {
					inPackets.push_back(message);
				}
				mutex.unlock();
				//on receive
				asyncReceive();
			}
		});
	}
	catch (std::exception e) {
		Logger::error("Failed to start async_receive OSC Message: {}", e.what());
	}
}
