#pragma once

#include "Networking/NetworkDevice.h"

#include "Project/Editor/Parameter.h"

#include <asio.hpp>

class LuaScript;

class ArtNode;


struct ArtDmx;


class DmxUniverse{
public:
	DmxUniverse(uint16_t universeNumber);
	void setChannel(int channel, uint8_t value);
	uint8_t* getBuffer();
	size_t getBufferSize();
private:
	std::shared_ptr<ArtNode> artnode;
	ArtDmx* artdmx;
	std::vector<uint8_t> buffer;
	size_t bufferSize = 1024;
};


class ArtNetNode : public NetworkDevice{
	DEFINE_NETWORK_DEVICE(ArtNetNode, "ArtNet Node", "ArtNetNode")
	
	virtual void nodeSpecificGui() override;
	virtual bool load(tinyxml2::XMLElement* xml) override;
	virtual bool save(tinyxml2::XMLElement* xml) override;
	
	std::unique_ptr<asio::ip::udp::socket> udpSocket;
	
	std::shared_ptr<LuaScript> script;
	
	void networkSettingsGui();
	void scriptingGui();
	
	void start();
	void stop();
	void sendDMX();
	
	std::vector<std::shared_ptr<DmxUniverse>> universes;
	std::shared_ptr<DmxUniverse> createNewUniverse(uint16_t universeNumber);
	void removeAllUniverses();
	
	bool b_running = false;
	bool b_isSending = false;
	
	std::shared_ptr<NumberParameter<uint8_t>> ipAddress0 = NumberParameter<uint8_t>::make(192, "ArtNet IP Octet 0", "IpOctet0");
	std::shared_ptr<NumberParameter<uint8_t>> ipAddress1 = NumberParameter<uint8_t>::make(168, "ArtNet IP Octet 1", "IpOctet1");
	std::shared_ptr<NumberParameter<uint8_t>> ipAddress2 = NumberParameter<uint8_t>::make(1, "ArtNet IP Octet 2", "IpOctet2");
	std::shared_ptr<NumberParameter<uint8_t>> ipAddress3 = NumberParameter<uint8_t>::make(33, "ArtNet IP Octet 3", "IpOctet3");
	std::shared_ptr<NumberParameter<uint16_t>> portNumber = NumberParameter<uint16_t>::make(6454, "ArtNet Port Number", "Port");
	std::shared_ptr<BooleanParameter> broadcast = BooleanParameter::make(false, "ArtNet Broadcast", "Broadcast");
	
	std::shared_ptr<NumberParameter<uint8_t>> networkIpAddress0 = NumberParameter<uint8_t>::make(192, "ArtNet Network IP Octet 0", "NetworkIpOctet0");
	std::shared_ptr<NumberParameter<uint8_t>> networkIpAddress1 = NumberParameter<uint8_t>::make(168, "ArtNet Network IP Octet 1", "NetworkIpOctet1");
	std::shared_ptr<NumberParameter<uint8_t>> networkIpAddress2 = NumberParameter<uint8_t>::make(0, "ArtNet Network IP Octet 2", "NetworkIpOctet2");
	std::shared_ptr<NumberParameter<uint8_t>> networkIpAddress3 = NumberParameter<uint8_t>::make(0, "ArtNet Network IP Octet 3", "NetworkIpOctet3");
	std::shared_ptr<NumberParameter<uint8_t>> networkMask0 = NumberParameter<uint8_t>::make(255, "ArtNet Network Mask Octet 0", "NetworkMaskOctet0");
	std::shared_ptr<NumberParameter<uint8_t>> networkMask1 = NumberParameter<uint8_t>::make(255, "ArtNet Network Mask Octet 1", "NetworkMaskOctet1");
	std::shared_ptr<NumberParameter<uint8_t>> networkMask2 = NumberParameter<uint8_t>::make(255, "ArtNet Network Mask Octet 2", "NetworkMaskOctet2");
	std::shared_ptr<NumberParameter<uint8_t>> networkMask3 = NumberParameter<uint8_t>::make(0, "ArtNet Network Mask Octet 3", "NetworkMaskOctet3");
	
	
	
	std::shared_ptr<NumberParameter<double>> sendingFrequency = NumberParameter<double>::make(1.0, "Sending frequency", "SendingFrequency", "%.1f", Units::Frequency::Hertz, false);
	
	void resetToDefaultArtNetPort(){ portNumber->overwriteWithHistory(6454); }
	
};


	
