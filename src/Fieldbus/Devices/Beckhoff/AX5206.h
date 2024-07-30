#pragma once

#include "Submodules/EL722xActuator.h"

class AX5206 : public EtherCatDevice{
public:
	DEFINE_ETHERCAT_DEVICE(AX5206, "AX5206", "AX5206", "Beckhoff", "Servo Drives", 0x2, 0x14566012)
			
};
