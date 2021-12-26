#pragma once

#include "NodeGraph/Device.h"


struct OscData{
	enum class Type{
		FLOAT_DATA,
		DOUBLE_DATA,
		INTEGER_DATA,
		BOOLEAN_DATA
	};
	Type type;
	char saveName[64];
	char displayName[64];
};

struct OscMessage{
	char address[256] = "/Stacato/";
	std::vector<OscData> arguments;
	uint64_t timestamp = 0;
};

class OscNode : public Device{
	
	DEFINE_DEVICE_NODE("Osc Node", OscNode, Device::Type::NETWORK_DEVICE, "Networking")
	
	virtual bool isDetected();
	
	virtual void assignIoData();
	
	virtual void readInputs();
	virtual void prepareOutputs();
	virtual void onConnection();
	virtual void onDisconnection();
	
	virtual void nodeSpecificGui();
	
};


