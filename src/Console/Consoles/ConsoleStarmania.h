#pragma once

#include "Console/ConsoleMapping.h"

class ConsoleStarmania : public ConsoleMapping, public std::enable_shared_from_this<ConsoleStarmania>{
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
		static std::string consoleName = "Stacato Starmania";
		return consoleName;
	}
	
	virtual void apply(std::shared_ptr<Console> console) override;
	
	virtual void gui(float height) override;
	
	std::shared_ptr<Joystick2X> joystickLeft;
	std::shared_ptr<Joystick2X> joystickRight;
	std::shared_ptr<LED_RGB_Button> joystickButtonLeft;
	std::shared_ptr<LED_RGB_Button> joystickButtonRight;
	
};
