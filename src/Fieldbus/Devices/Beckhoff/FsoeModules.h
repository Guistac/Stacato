#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/FsoeConnection.h"

class EL2912 : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(EL2912, "EL2912 2x FsoE Digital Output", "EL2912", "Beckhoff", "Utilities", 0x2, 0xb603052)

	std::shared_ptr<bool> out1 = std::make_shared<bool>(false);
	std::shared_ptr<bool> out2 = std::make_shared<bool>(false);
	
	uint8_t safe_inputs = 0;
	uint8_t safe_outputs = 0;
	uint16_t fsoeAddress = 0x2;

	struct ProcessData{
		bool safeOutput1 = false;
		bool safeOutput2 = false;
		bool safeOutput1ErrAck = false;
		bool safeOutput2ErrAck = false;
		bool safeOutput1Fault = false;
		bool safeOutput2Fault = false;
	}processData;

	void downladSafetyParameters();
	
	FsoeConnection fsoeConnection;
};

class EL1904 : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(EL1904, "EL1904 4x FsoE Digital Input", "EL1904", "Beckhoff", "Utilities", 0x2, 0x7703052)

	std::shared_ptr<bool> in1 = std::make_shared<bool>(false);
	std::shared_ptr<bool> in2 = std::make_shared<bool>(false);
	std::shared_ptr<bool> in3 = std::make_shared<bool>(false);
	std::shared_ptr<bool> in4 = std::make_shared<bool>(false);
	
	struct ProcessData{
		uint8_t safe_inputs = 0;
		uint8_t safe_outputs = 0;
	}processData;
	
	uint16_t fsoeAddress = 0x3;

	void downladSafetyParameters();
	
	FsoeConnection fsoeConnection;
};
