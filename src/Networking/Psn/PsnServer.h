#pragma once

#include "Networking/NetworkDevice.h"

#include "Project/Editor/Parameter.h"

#include <asio.hpp>
#include <psn_defs.hpp>
#include <psn_encoder.hpp>

class LuaScript;

class PsnServer : public NetworkDevice{
	DEFINE_NETWORK_DEVICE(PsnServer, "PSN Server", "PsnServer")
	
	virtual void nodeSpecificGui() override;
	virtual bool load(tinyxml2::XMLElement* xml) override;
	virtual bool save(tinyxml2::XMLElement* xml) override;
	
	std::unique_ptr<asio::ip::udp::socket> udpSocket;
	bool b_online = false;
	
	std::shared_ptr<psn::psn_encoder> psnEncoder;
	psn::tracker_map trackers;
	
	bool startServer();
	void stopServer();
	bool b_serverRunning = false;
	
	void sendInfo();
	void sendData();
	
	std::shared_ptr<StringParameter> serverName = StringParameter::make("Stacato PSN Server", "PSN Server Name", "ServerName", 512);
	
	std::shared_ptr<NumberParameter<double>> infoSendingFrequency = NumberParameter<double>::make(1.0,
																								  "Info Packet sending frequency",
																								  "InfoPacketSendingFrequency",
																								  "%.1f",
																								  Units::Frequency::Hertz,
																								  false);
	
	std::shared_ptr<NumberParameter<double>> dataSendingFrequency = NumberParameter<double>::make(60.0,
																								  "Data Packet sending frequency",
																								  "DataPacketSendingFrequency",
																								  "%.1f",
																								  Units::Frequency::Hertz,
																								  false);
	std::shared_ptr<NumberParameter<uint8_t>> ipAddress0 = NumberParameter<uint8_t>::make(236, "PSN IP Octet 0", "IpOctet0");
	std::shared_ptr<NumberParameter<uint8_t>> ipAddress1 = NumberParameter<uint8_t>::make(10, "PSN IP Octet 1", "IpOctet1");
	std::shared_ptr<NumberParameter<uint8_t>> ipAddress2 = NumberParameter<uint8_t>::make(10, "PSN IP Octet 2", "IpOctet2");
	std::shared_ptr<NumberParameter<uint8_t>> ipAddress3 = NumberParameter<uint8_t>::make(10, "PSN IP Octet 3", "IpOctet3");
	std::shared_ptr<NumberParameter<uint16_t>> portNumber = NumberParameter<uint16_t>::make(56565, "PSN Port Number", "Port");
	
	std::shared_ptr<LuaScript> script;
	
	bool b_addressIsMulticast = false;
	void setDefaultAddressSettings();
	
	void networkSettingsGui();
	void scriptingGui();
};


	
