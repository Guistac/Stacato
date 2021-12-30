#pragma once

#include "NetworkDevice.h"

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

class OscSocket : public NetworkDevice{
	DEFINE_NETWORK_DEVICE(OscSocket, "Osc Socket", "OscSocket")
};


