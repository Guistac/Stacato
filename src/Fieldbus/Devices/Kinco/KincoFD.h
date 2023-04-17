#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/DS402Axis.h"

class KincoFD : public EtherCatDevice {
public:

	DEFINE_ETHERCAT_DEVICE(KincoFD, "Kinco", "Servo Drives", 0x681168, 0x464445)

	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		EtherCatDevice::onCopyFrom(source);
	};
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
	std::shared_ptr<DS402Axis> axis;
	
	bool b_enable = false;
	bool b_disable = false;
	uint32_t pos = 0;
	
};
