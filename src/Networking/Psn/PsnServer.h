#pragma once

#include "Networking/NetworkDevice.h"

#include "Project/Editor/Parameter.h"

#include <asio.hpp>
#include <psn_defs.hpp>
#include <psn_encoder.hpp>

class LuaScript;


class PsnTracker {
public:
	
	PsnTracker(psn::tracker& t) : psncpptracker(&t){}
	
	void setPosition(glm::vec3 position);
	void setVelocity(glm::vec3 velocity);
	void setAcceleration(glm::vec3 acceleration);
	void setTarget(glm::vec3 targetPosition);
	void setOrigin(glm::vec3 origin);
	void setStatus(float status);
	void setTimestamp(uint64_t timestamp);
	psn::tracker* psncpptracker;
};


class PsnServer : public NetworkDevice{
	DEFINE_NETWORK_DEVICE(PsnServer, "PSN Server", "PsnServer")
	
	virtual void nodeSpecificGui() override;
	virtual bool load(tinyxml2::XMLElement* xml) override;
	virtual bool save(tinyxml2::XMLElement* xml) override;
	
	std::shared_ptr<PsnTracker> createNewTracker(std::string name);
	
private:
	
	std::unique_ptr<asio::ip::udp::socket> udpSocket;
	std::shared_ptr<psn::psn_encoder> psnEncoder;
	bool b_online = false;
	bool b_isSending = false;
	
	void removeAllTrackers();
	psn::tracker_map trackers;
	uint16_t trackerCount = 0;
	std::mutex trackerMutex;
	
	bool startServer();
	void stopServer();
	bool b_serverRunning = false;
	long long serverEnvironnementStartTimeMicroseconds;
	long long serverProgramStartTimeMicroseconds;
	
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
	std::shared_ptr<NumberParameter<uint8_t>> destinationIp0 = NumberParameter<uint8_t>::make(236, "PSN Destination IP Octet 0", "DestinationIpOctet0");
	std::shared_ptr<NumberParameter<uint8_t>> destinationIp1 = NumberParameter<uint8_t>::make(10, "PSN Destination IP Octet 1", "DestinationIpOctet1");
	std::shared_ptr<NumberParameter<uint8_t>> destinationIp2 = NumberParameter<uint8_t>::make(10, "PSN Destination IP Octet 2", "DestinationIpOctet2");
	std::shared_ptr<NumberParameter<uint8_t>> destinationIp3 = NumberParameter<uint8_t>::make(10, "PSN Destination IP Octet 3", "DestinationIpOctet3");
	std::shared_ptr<NumberParameter<uint16_t>> destinationPortNumber = NumberParameter<uint16_t>::make(56565, "PSN Destination Port Number", "Port");
	
	std::shared_ptr<NumberParameter<uint8_t>> localIp0 = NumberParameter<uint8_t>::make(192, "PSN Local IP Octet 0", "LocalIpOctet0");
	std::shared_ptr<NumberParameter<uint8_t>> localIp1 = NumberParameter<uint8_t>::make(168, "PSN Local IP Octet 1", "LocalIpOctet1");
	std::shared_ptr<NumberParameter<uint8_t>> localIp2 = NumberParameter<uint8_t>::make(0, "PSN Local IP Octet 2", "LocalIpOctet2");
	std::shared_ptr<NumberParameter<uint8_t>> localIp3 = NumberParameter<uint8_t>::make(0, "PSN Local IP Octet 3", "LocalIpOctet3");
	
	std::shared_ptr<LuaScript> script;
	
	bool b_addressIsMulticast = false;
	void setDefaultAddressSettings();
	
	void networkSettingsGui();
	void scriptingGui();
};


	
