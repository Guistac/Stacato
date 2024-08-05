#include <pch.h>
#include "ConsoleIODevice.h"

std::shared_ptr<PushButton>		IODevice::toPushButton(){ 		return std::dynamic_pointer_cast<PushButton>(shared_from_this()); }
std::shared_ptr<Joystick2X>		IODevice::toJoystick2X(){ 		return std::dynamic_pointer_cast<Joystick2X>(shared_from_this()); }
std::shared_ptr<Joystick3X>		IODevice::toJoystick3X(){ 		return std::dynamic_pointer_cast<Joystick3X>(shared_from_this()); }
std::shared_ptr<LED>			IODevice::toLED(){ 				return std::dynamic_pointer_cast<LED>(shared_from_this()); }
std::shared_ptr<LED_PWM>		IODevice::toLED_PWM(){ 			return std::dynamic_pointer_cast<LED_PWM>(shared_from_this()); }
std::shared_ptr<LED_RGB>		IODevice::toLED_RGB(){ 			return std::dynamic_pointer_cast<LED_RGB>(shared_from_this()); }
std::shared_ptr<LED_Button>		IODevice::toLED_Button(){ 		return std::dynamic_pointer_cast<LED_Button>(shared_from_this()); }
std::shared_ptr<LED_PWM_Button>	IODevice::toLED_PWM_Button(){ 	return std::dynamic_pointer_cast<LED_PWM_Button>(shared_from_this()); }
std::shared_ptr<LED_RGB_Button>	IODevice::toLED_RGB_Button(){ 	return std::dynamic_pointer_cast<LED_RGB_Button>(shared_from_this()); }

std::shared_ptr<IODevice> IODevice::make(Type deviceType){
	switch(deviceType){
		case Type::PUSHBUTTON:		return std::make_shared<PushButton>();
		case Type::SWITCH:			return nullptr;
		case Type::POTENTIOMETER:	return nullptr;
		case Type::ENCODER:			return nullptr;
		case Type::JOYSTICK_1AXIS:	return nullptr;
		case Type::JOYSTICK_2AXIS:	return std::make_shared<Joystick2X>();
		case Type::JOYSTICK_3AXIS:	return std::make_shared<Joystick3X>();
		case Type::LED:				return std::make_shared<LED>();
		case Type::LED_PWM:			return std::make_shared<LED_PWM>();
		case Type::LED_RGB:			return std::make_shared<LED_RGB>();
		case Type::LED_BUTTON:		return std::make_shared<LED_Button>();
		case Type::LED_PWM_BUTTON:	return std::make_shared<LED_PWM_Button>();
		case Type::LED_RGB_BUTTON:	return std::make_shared<LED_RGB_Button>();
		case Type::UNKNOWN:			return nullptr;
	}
}

uint8_t IODevice::getCodeFromType(Type deviceType){
	switch(deviceType){
		case Type::PUSHBUTTON:    	return 0x10;
		case Type::SWITCH:        	return 0x11;
		case Type::POTENTIOMETER: 	return 0x12;
		case Type::ENCODER:       	return 0x13;
		case Type::JOYSTICK_1AXIS:	return 0x14;
		case Type::JOYSTICK_2AXIS:	return 0x15;
		case Type::JOYSTICK_3AXIS:	return 0x16;
		case Type::LED:           	return 0x17;
		case Type::LED_PWM:       	return 0x18;
		case Type::LED_RGB:       	return 0x19;
		case Type::LED_BUTTON:    	return 0x1A;
		case Type::LED_PWM_BUTTON:	return 0x1B;
		case Type::LED_RGB_BUTTON:	return 0x1C;
		default: 					return 0xFF;
	}
}

IODevice::Type IODevice::getTypeFromCode(uint8_t deviceTypeCode){
	switch(deviceTypeCode){
		case 0x10: 	return Type::PUSHBUTTON;
		case 0x11: 	return Type::SWITCH;
		case 0x12: 	return Type::POTENTIOMETER;
		case 0x13: 	return Type::ENCODER;
		case 0x14: 	return Type::JOYSTICK_1AXIS;
		case 0x15: 	return Type::JOYSTICK_2AXIS;
		case 0x16: 	return Type::JOYSTICK_3AXIS;
		case 0x17: 	return Type::LED;
		case 0x18: 	return Type::LED_PWM;
		case 0x19: 	return Type::LED_RGB;
		case 0x1A: 	return Type::LED_BUTTON;
		case 0x1B: 	return Type::LED_PWM_BUTTON;
		case 0x1C: 	return Type::LED_RGB_BUTTON;
		default: 	return Type::UNKNOWN;
	}
}

const char* IODevice::getTypeString(Type deviceType){
	switch(deviceType){
		case Type::PUSHBUTTON:    	return "PushButton";
		case Type::SWITCH:        	return "Switch";
		case Type::POTENTIOMETER: 	return "Potentiometer";
		case Type::ENCODER:       	return "Encoder";
		case Type::JOYSTICK_1AXIS:	return "Joystick 1 Axis";
		case Type::JOYSTICK_2AXIS:	return "Joystick 2 Axis";
		case Type::JOYSTICK_3AXIS:	return "Joystick 3 Axis";
		case Type::LED:           	return "LED";
		case Type::LED_PWM:       	return "LED (PWM)";
		case Type::LED_RGB:       	return "LED (RGB)";
		case Type::LED_BUTTON:    	return "Button with LED";
		case Type::LED_PWM_BUTTON:	return "Button with LED (PWM)";
		case Type::LED_RGB_BUTTON:	return "Button with LED (RGB)";
		default: 					return "Unknown Device";
	}
}
