#pragma once

#include "Console/ConsoleMapping.h"

class ButtonBoardMapping : public ConsoleMapping{
public:
	
	virtual std::vector<IODevice::Type>& getDeviceTypes() override{
		static std::vector<IODevice::Type> deviceTypes = {
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON
		};
		return deviceTypes;
	}
	
	virtual std::string& getConsoleName() override{
		static std::string consoleName = "8x Button Board";
		return consoleName;
	}
	
	virtual void apply(std::shared_ptr<Console> console) override;
	
	
};
