#pragma once

#include "Fieldbus/EtherCatDevice.h"

class ECAT_2511_A_FiberConverter : public EtherCatDevice {
public:
    DEFINE_ETHERCAT_DEVICE(ECAT_2511_A_FiberConverter, "ECAT-2511-A RJ45 to Fiber EtherCAT Converter [A]", "ECAT-2511-A", "ICPDAS", "Utilities", 0x494350, 0x9cf)
};

class ECAT_2511_B_FiberConverter : public EtherCatDevice {
public:
    DEFINE_ETHERCAT_DEVICE(ECAT_2511_B_FiberConverter, "ECAT-2511-A Fiber to RJ45 EtherCAT Converter [B]", "ECAT-2511-B", "ICPDAS", "Utilities", 0x494350, 0x9ce)
};
                                
class ECAT_2515_6PortJunction : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(ECAT_2515_6PortJunction, "ECAT-2515 6-Port EtherCAT Junction", "ECAT-2515", "ICPDAS", "Utilities", 0x494350, 0x9d3)
};

class ECAT_2517_7PortJunction : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(ECAT_2517_7PortJunction, "ECAT-2517 7-Port EtherCAT Junction", "ECAT-2517", "ICPDAS", "Utilities", 0x494350, 0x9d5)
};
