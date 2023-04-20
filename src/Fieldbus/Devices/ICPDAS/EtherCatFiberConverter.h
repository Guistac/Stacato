#pragma once

#include "Fieldbus/EtherCatDevice.h"

class ECAT_2511_A_FiberConverter : public EtherCatDevice {
public:
    DEFINE_ETHERCAT_DEVICE(ECAT_2511_A_FiberConverter, "ICPDAS", "Utilities", 0x494350, 0x9cf)
	virtual void onConstruction() override{
		EtherCatDevice::onConstruction();
		setName("ECAT-2511-A RJ45 to Fiber EtherCAT Converter [A]");
	}
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
		EtherCatDevice::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		return EtherCatDevice::onSerialization();
	}
	virtual bool onDeserialization() override{
		return EtherCatDevice::onDeserialization();
	}
};

class ECAT_2511_B_FiberConverter : public EtherCatDevice {
public:
    DEFINE_ETHERCAT_DEVICE(ECAT_2511_B_FiberConverter, "ICPDAS", "Utilities", 0x494350, 0x9ce)
	virtual void onConstruction() override{
		EtherCatDevice::onConstruction();
		setName("ECAT-2511-A Fiber to RJ45 EtherCAT Converter [B]");
	}
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
		EtherCatDevice::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		return EtherCatDevice::onSerialization();
	}
	virtual bool onDeserialization() override{
		return EtherCatDevice::onDeserialization();
	}
};
                                
class ECAT_2515_6PortJunction : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(ECAT_2515_6PortJunction, "ICPDAS", "Utilities", 0x494350, 0x9d3)
	
	virtual void onConstruction() override{
		EtherCatDevice::onConstruction();
		setName("ECAT-2515 6-Port EtherCAT Junction");
	}
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override{
		EtherCatDevice::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		return EtherCatDevice::onSerialization();
	}
	virtual bool onDeserialization() override{
		return EtherCatDevice::onDeserialization();
	}
};



