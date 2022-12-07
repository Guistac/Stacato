#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402Axis.h"

class ATV340 : public EtherCatDevice {
public:

	DEFINE_ETHERCAT_DEVICE(ATV340, "ATV340", "ATV340", "Schneider Electric", "Servo Drives", 0x800005A, 0x12C)
	
	std::shared_ptr<DS402Axis> axis;
	uint16_t motorPower;
	uint16_t logicInputs;
	uint16_t stoState;
	uint16_t lastFaultCode;
	
	bool configureMotor();
	
};
