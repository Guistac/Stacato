#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402.h"

class ATV320 : public EtherCatDevice{
public:
	
	DEFINE_ETHERCAT_DEVICE(ATV320, "ATV320 EtherCAT", "ATV320", "ATV320", "Schneider Electric", "Motor Drives")
	
	DS402::Status ds402SStatus;
	DS402::Control ds402Control;
	
	
};
