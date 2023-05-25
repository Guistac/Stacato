#pragma once

#include "Console/ConsoleMapping.h"

class StacatoCompact : public ConsoleMapping, public std::enable_shared_from_this<StacatoCompact>{
public:
	
	virtual std::vector<IODevice::Type>& getDeviceTypes() override{
		static std::vector<IODevice::Type> deviceTypes = {
			IODevice::Type::LED_PWM,
			IODevice::Type::LED_PWM,
			IODevice::Type::LED_PWM,
			IODevice::Type::LED_PWM,
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
			IODevice::Type::PUSHBUTTON,
			IODevice::Type::PUSHBUTTON,
			IODevice::Type::JOYSTICK_2AXIS
		};
		return deviceTypes;
	}
	
	virtual std::string& getConsoleName() override{
		static std::string consoleName = "StacatoCompact";
		return consoleName;
	}
	
	virtual void apply(std::shared_ptr<Console> console) override;
	
	virtual void gui(float height) override;
	
	std::shared_ptr<LED_PWM> pwmLed1;
	std::shared_ptr<LED_PWM> pwmLed2;
	std::shared_ptr<LED_PWM> pwmLed3;
	std::shared_ptr<LED_PWM> pwmLed4;
	std::shared_ptr<LED_RGB_Button> rgbButton1;
	std::shared_ptr<LED_RGB_Button> rgbButton2;
	std::shared_ptr<LED_RGB_Button> rgbButton3;
	std::shared_ptr<LED_RGB_Button> rgbButton4;
	std::shared_ptr<LED_RGB_Button> rgbButton5;
	std::shared_ptr<LED_RGB_Button> rgbButton6;
	std::shared_ptr<LED_RGB_Button> rgbButton7;
	std::shared_ptr<LED_RGB_Button> rgbButton8;
	std::shared_ptr<PushButton> pushButton1;
	std::shared_ptr<PushButton> pushButton2;
	std::shared_ptr<PushButton> pushButton3;
	std::shared_ptr<PushButton> pushButton4;
	std::shared_ptr<PushButton> pushButton5;
	std::shared_ptr<Joystick2X> joystick;
	
	virtual void onDisconnection() override;
	
};
