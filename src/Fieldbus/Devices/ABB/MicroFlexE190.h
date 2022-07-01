#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

#include "Fieldbus/Utilities/DS402.h"

class MicroFlex_e190 : public EtherCatDevice {
public:

    DEFINE_ETHERCAT_DEVICE(MicroFlex_e190, "MicroFlex e190", "MicroFlex_e190", "MicroFlex_e190", "ABB", "Servo Drives", 0xB7, 0x2C1)
	
	DS402::Status ds402Status;
	DS402::Control ds402Control;
	int32_t targetPosition;
	int32_t actualPosition;

	DS402::PowerState actualPowerState = DS402::PowerState::UNKNOWN;
	DS402::PowerState requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
	
	DS402::OperatingMode actualOperatingMode = DS402::OperatingMode::UNKNOWN;
	DS402::OperatingMode requestedOperatingMode = DS402::OperatingMode::PROFILE_VELOCITY;
	
	void controlTab();
	
};
