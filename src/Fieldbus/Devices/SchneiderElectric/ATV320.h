#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402.h"

class ATV320 : public EtherCatDevice{
public:
	
	DEFINE_ETHERCAT_DEVICE(ATV320, "ATV320", "ATV320", "Schneider Electric", "Motor Drives", 0x800005A, 0x0)
	
	DS402::Status ds402SStatus;
	DS402::Control ds402Control;
	
	DS402::PowerState powerStateTarget = DS402::PowerState::SWITCH_ON_DISABLED;
	DS402::PowerState powerStateActual = DS402::PowerState::UNKNOWN;
	
	bool b_resetFaultCommand = false;
	
	bool b_velocityTargetReached = false;
	
	int16_t velocityTarget = 0;
	int16_t velocityActual = 0;
	
	uint16_t logicInputs = 0;
	//uint16_t logicOutputs = 0;
	
	float accelerationTime_seconds = 3.0;
	float decelerationTime_seconds = 3.0;
	
};
