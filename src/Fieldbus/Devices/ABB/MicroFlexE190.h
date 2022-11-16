#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

#include "Fieldbus/Utilities/DS402Axis.h"

class MicroFlex_e190 : public EtherCatDevice {
public:

    DEFINE_ETHERCAT_DEVICE(MicroFlex_e190, "MicroFlex e190", "MicroFlex_e190", "ABB", "Servo Drives", 0xB7, 0x2C1)
	
	DS402Axis axis;
	int16_t SYS_GlobalErrorOutput_I16;
	uint16_t AX0_errorCode_U16;
	
	void controlTab();
	
	double axisUnitsPerPos = 100.0;
	double axisUnitsPerVel = 100.0;
	
	double acceleration = 30.0;
	double velocity = 0.0;
	double position = 0.0;
	
	double maxVelocity = 100.0;
	
	float manualVelocity = 0.0;
	
};
