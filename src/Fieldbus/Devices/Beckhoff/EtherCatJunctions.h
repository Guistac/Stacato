#pragma once

#include "Fieldbus/EtherCatDevice.h"

class CU1128 : public EtherCatDevice {
public:
    DEFINE_ETHERCAT_DEVICE(CU1128, "Beckhoff", "Utilities", 0x2, 0x4685432)
	virtual void onConstruction() override{
		EtherCatDevice::onConstruction();
		setName("CU1128 8-Port EtherCAT Junction");
	}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		EtherCatDevice::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		return EtherCatDevice::onSerialization();
	}
	virtual bool onDeserialization() override{
		return EtherCatDevice::onDeserialization();
	}
};

class CU1124 : public EtherCatDevice {
public:
	DEFINE_ETHERCAT_DEVICE(CU1124, "Beckhoff", "Utilities", 0x2, 0x4645432)
	virtual void onConstruction() override{
		EtherCatDevice::onConstruction();
		setName("CU1124 4-Port EtherCAT Junction");
	}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		EtherCatDevice::onCopyFrom(source);
	}
	virtual bool onSerialization() override{
		return EtherCatDevice::onSerialization();
	}
	virtual bool onDeserialization() override{
		return EtherCatDevice::onDeserialization();
	}
};
