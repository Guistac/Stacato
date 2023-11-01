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
    
    std::shared_ptr<LED_RGB_Button> left_rgb_button;
    std::shared_ptr<LED_RGB_Button> right_rgb_button;
    
    std::shared_ptr<LED_RGB_Button> goleft_rgb_Button;
    std::shared_ptr<LED_RGB_Button> stop_rgb_Button;
    std::shared_ptr<LED_RGB_Button> play_rgb_Button;
    std::shared_ptr<LED_RGB_Button> goright_rgb_Button;
    
    std::shared_ptr<PushButton> left_selection_button;
    std::shared_ptr<PushButton> right_selection_button;
    std::shared_ptr<PushButton> rearm_button;
    
    std::shared_ptr<LED_RGB_Button> square_rgb_button_1;
    std::shared_ptr<LED_RGB_Button> square_rgb_button_2;
    std::shared_ptr<LED_RGB_Button> square_rgb_button_3;
    std::shared_ptr<LED_RGB_Button> square_rgb_button_4;
    std::shared_ptr<LED_RGB_Button> square_rgb_button_5;
    std::shared_ptr<LED_RGB_Button> square_rgb_button_6;
    std::shared_ptr<LED_RGB_Button> square_rgb_button_7;
    std::shared_ptr<LED_RGB_Button> square_rgb_button_8;
	
	virtual void onDisconnection() override;
	
};
