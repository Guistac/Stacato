#pragma once

#include "Console/ConsoleMapping.h"
#include "Animation/ManualControlChannel.h"

class StacatoV2 : public ConsoleMapping, public std::enable_shared_from_this<StacatoV2>{
public:
	
	virtual std::vector<IODevice::Type>& getDeviceTypes() override{
		static std::vector<IODevice::Type> deviceTypes = {
			IODevice::Type::LED_PWM,
			IODevice::Type::LED_PWM,
			IODevice::Type::LED_PWM,
			IODevice::Type::LED_RGB,
			IODevice::Type::LED_RGB,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::LED_RGB_BUTTON,
			IODevice::Type::PUSHBUTTON,
			IODevice::Type::PUSHBUTTON,
			IODevice::Type::JOYSTICK_3AXIS,
			IODevice::Type::JOYSTICK_3AXIS,
			IODevice::Type::PUSHBUTTON,
			IODevice::Type::PUSHBUTTON
		};
		return deviceTypes;
	}
	
	virtual std::string& getConsoleName() override{
		static std::string consoleName = "StacatoV2";
		return consoleName;
	}
	
	virtual void apply(std::shared_ptr<Console> console) override;
	
	virtual void gui(float height) override;
	
	std::shared_ptr<LED_PWM> led_EtherCat;
	std::shared_ptr<LED_PWM> led_Light;
	std::shared_ptr<LED_PWM> led_Video;
	std::shared_ptr<LED_RGB> rgbLed_leftDeadman;
	std::shared_ptr<LED_RGB> rgbLed_rightDeadman;
	std::shared_ptr<LED_RGB_Button> rgbButton_goToStart;
	std::shared_ptr<LED_RGB_Button> rgbButton_PlayPause;
	std::shared_ptr<LED_RGB_Button> rgbButton_Stop;
	std::shared_ptr<LED_RGB_Button> rgbButton_goToEnd;
	std::shared_ptr<PushButton> pushbutton_arrowUp;
	std::shared_ptr<PushButton> pushbutton_arrowDown;
	std::shared_ptr<Joystick3X> joystick3x_left;
	std::shared_ptr<Joystick3X> joystick3x_right;
	std::shared_ptr<PushButton> pushbutton_leftJoystickDeadman;
	std::shared_ptr<PushButton> pushbutton_rightJoystickDeadman;
	
	virtual void onDisconnection() override;
	
	std::shared_ptr<ManualControlChannel> leftJoystickControlChannel = std::make_shared<ManualControlChannel>();
	std::shared_ptr<ManualControlChannel> rightJoystickControlChannel = std::make_shared<ManualControlChannel>();
	
};
