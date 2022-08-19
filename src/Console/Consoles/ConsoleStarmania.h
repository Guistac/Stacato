#pragma once

#include "Console/ConsoleMapping.h"

class ConsoleStarmaniaMapping : public ConsoleMapping{
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
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::PUSHBUTTON,
			IODevice::Type::PUSHBUTTON,
			IODevice::Type::PUSHBUTTON,
			IODevice::Type::JOYSTICK_2AXIS,
			IODevice::Type::JOYSTICK_2AXIS
		};
		return deviceTypes;
	}
	
	virtual std::string& getConsoleName() override{
		static std::string consoleName = "Console Starmania";
		return consoleName;
	}
	
	virtual void apply(std::shared_ptr<Console> console) override;
	
	
};
