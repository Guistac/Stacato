#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

#include "Fieldbus/Utilities/DS402.h"

class IL_EC_BK_BusCoupler : public EtherCatDevice {
public:

    DEFINE_ETHERCAT_DEVICE(IL_EC_BK_BusCoupler, "Phoenix Contact Bus Coupler", "IL_EC_BK_BusCoupler", "Phoenix Contact", "I/O", 0x84, 0x293CAB)
	
};
