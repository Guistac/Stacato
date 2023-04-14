#pragma once

#include "Networking/NetworkDevice.h"

#include "Legato/Editor/Parameters.h"

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
	DEFINE_NETWORK_DEVICE(PsnServer)
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		NetworkDevice::onCopyFrom(source);
	};
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
public:

	virtual void nodeSpecificGui() override;
	
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
	
	Legato::StringParam serverName;
	
	Legato::NumberParam<double> infoSendingFrequency;
	Legato::NumberParam<double> dataSendingFrequency;
																								
	Legato::NumberParam<uint8_t> destinationIp0;
	Legato::NumberParam<uint8_t> destinationIp1;
	Legato::NumberParam<uint8_t> destinationIp2;
	Legato::NumberParam<uint8_t> destinationIp3;
	Legato::NumberParam<uint16_t> destinationPortNumber;
	
	Legato::NumberParam<uint8_t> localIp0;
	Legato::NumberParam<uint8_t> localIp1;
	Legato::NumberParam<uint8_t> localIp2;
	Legato::NumberParam<uint8_t> localIp3;
	
	std::shared_ptr<LuaScript> script;
	
	bool b_addressIsMulticast = false;
	void setDefaultAddressSettings();
	
	void networkSettingsGui();
	void scriptingGui();
};


	
