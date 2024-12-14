#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/FsoeConnection.h"

class EL2912 : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(EL2912, "EL2912 2x FsoE Digital Output", "EL2912", "Beckhoff", "Utilities", 0x2, 0xb603052)

	uint8_t safe_inputs = 0;
	uint8_t safe_outputs = 0;
	uint16_t fsoeAddress = 0x2;

	bool safeOutput1 = false;
	bool safeOutput2 = false;
	bool safeOutput1ErrAck = false;
	bool safeOutput2ErrAck = false;

	bool safeOutput1Fault = false;
	bool safeOutput2Fault = false;

	void downladSafetyParameters();
	
	FsoeConnection fsoeConnection;
};

class EL1904 : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(EL1904, "EL1904 4x FsoE Digital Input", "EL1904", "Beckhoff", "Utilities", 0x2, 0x7703052)

	uint8_t safe_inputs = 0;
	uint8_t safe_outputs = 0;
	uint16_t fsoeAddress = 0x3;

	bool safeInput1 = false;
	bool safeInput2 = false;
	bool safeInput3 = false;
	bool safeInput4 = false;

	void downladSafetyParameters();
	
	FsoeConnection fsoeConnection;
};
