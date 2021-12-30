#pragma once

#include "Networking/NetworkDevice.h"

class OscSocket;

/*
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
*/

class OscDevice : public NetworkDevice{
	DEFINE_NETWORK_DEVICE(OscDevice, "Osc Device", "OscDevice")
	
	void networkGui();
	void dataGui();
	
	bool b_enabled = false;
	
	std::shared_ptr<OscSocket> oscSocket;
	
	std::thread runtime;
	
	int remoteIP[4] = {0, 0, 0, 0};
	int remotePort = 0;
	int listeningPort = 0;
};


