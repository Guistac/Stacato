#pragma once

#include "Fieldbus/EtherCatDevice.h"

class ECAT_2511_A_FiberConverter : public EtherCatDevice {
public:
    DEFINE_ETHERCAT_DEVICE(ECAT_2511_A_FiberConverter, "ICPDAS", "Utilities", 0x494350, 0x9cf)
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		EtherCatDevice::onCopyFrom(source);
	}
};

class ECAT_2511_B_FiberConverter : public EtherCatDevice {
public:
    DEFINE_ETHERCAT_DEVICE(ECAT_2511_B_FiberConverter, "ICPDAS", "Utilities", 0x494350, 0x9ce)
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		EtherCatDevice::onCopyFrom(source);
	}
};
                                
class ECAT_2515_6PortJunction : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(ECAT_2515_6PortJunction, "ICPDAS", "Utilities", 0x494350, 0x9d3)
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		EtherCatDevice::onCopyFrom(source);
	}
};



