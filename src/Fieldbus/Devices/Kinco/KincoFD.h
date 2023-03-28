#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402.h"

class KincoFD : public EtherCatDevice {
public:
	
	DEFINE_ETHERCAT_DEVICE(KincoFD, "Kinco FD", "KincoFD", "Kinco", "Servo Drives", 0x681168, 0x464445)

};
