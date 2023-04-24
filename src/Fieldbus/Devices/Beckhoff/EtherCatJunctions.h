#pragma once

#include "Fieldbus/EtherCatDevice.h"

class CU1128 : public EtherCatDevice {
public:
    DEFINE_ETHERCAT_DEVICE(CU1128, "CU1128 8-Port EtherCAT Junction", "CU1128", "Beckhoff", "Utilities", 0x2, 0x4685432)
};

class CU1124 : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(CU1124, "CU1124 4-Port EtherCAT Junction", "CU1124", "Beckhoff", "Utilities", 0x2, 0x4645432)
};


class EK1100 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EK1100, "EK1100 Bus Coupler", "EK1100", "Beckhoff", "I/O", 0x2, 0x44c2c52)
};
 
class EL2008 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EL2008, "EL2008 8x Digital Output", "EL2008", "Beckhoff", "I/O", 0x2, 0x7d83052)
	bool outputs[8] = {0,0,0,0,0,0,0,0};
};

class EL5001 : public EtherCatDevice{
public:
DEFINE_ETHERCAT_DEVICE(EL5001, "EK1100 SSI Input", "EL5001", "Beckhoff", "I/O", 0x2, 0x13893052)
	//txPdo
	uint8_t status;
	uint32_t ssiValue;
};

