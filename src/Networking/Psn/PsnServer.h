#pragma once

#include "Networking/NetworkDevice.h"

class PsnServer : public NetworkDevice{
	DEFINE_NETWORK_DEVICE(PsnServer, "PSN Server", "PsnServer")
	
	virtual void nodeSpecificGui() override;
	virtual bool load(tinyxml2::XMLElement* xml) override;
	virtual bool save(tinyxml2::XMLElement* xml) override;
	
	
	/*
	 virtual Device::Type getDeviceType() { return Type::NETWORK_DEVICE; }\
	 virtual void readInputs();\
	 virtual void writeOutputs();\
	 virtual bool isDetected();\
	 virtual bool isConnected();\
	 virtual void onConnection();\
	 virtual void onDisconnection();\
	 \
	 virtual void connect();\
	 virtual void disconnect();\
	 */
};


	
