#pragma once

#include "Networking/NetworkDevice.h"

#include "Legato/Editor/Parameters.h"

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
	DEFINE_NETWORK_DEVICE(ArtNetNode)
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		NetworkDevice::onCopyFrom(source);
	};
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
	virtual void nodeSpecificGui() override;
	
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
	
	Legato::NumberParam<uint8_t> ipAddress0;
	Legato::NumberParam<uint8_t> ipAddress1;
	Legato::NumberParam<uint8_t> ipAddress2;
	Legato::NumberParam<uint8_t> ipAddress3;
	Legato::NumberParam<uint16_t> portNumber;
	Legato::BoolParam broadcast;
	
	Legato::NumberParam<uint8_t> networkIpAddress0;
	Legato::NumberParam<uint8_t> networkIpAddress1;
	Legato::NumberParam<uint8_t> networkIpAddress2;
	Legato::NumberParam<uint8_t> networkIpAddress3;
	Legato::NumberParam<uint8_t> networkMask0;
	Legato::NumberParam<uint8_t> networkMask1;
	Legato::NumberParam<uint8_t> networkMask2;
	Legato::NumberParam<uint8_t> networkMask3;
	
	
	
	std::shared_ptr<NumberParameter<double>> sendingFrequency = NumberParameter<double>::make(1.0, "Sending frequency", "SendingFrequency", "%.1f", Units::Frequency::Hertz, false);
	
	void resetToDefaultArtNetPort(){ portNumber->overwriteWithHistory(6454); }
	
};


	
