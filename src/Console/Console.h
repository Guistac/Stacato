#pragma once

namespace Console{
	
	void initialize();
	void terminate();
	
	bool isConnected();
	std::string& getName();

	enum class DeviceType{
		PUSHBUTTON,
		SWITCH,
		POTENTIOMETER,
		ENCODER,
		JOYSTICK_1AXIS,
		JOYSTICK_2AXIS,
		JOYSTICK_3AXIS,
		LED,
		LED_PWM,
		LED_RGB,
		LED_BUTTON,
		LED_PWM_BUTTON,
		LED_RGB_BUTTON,
		UNKNOWN
	};

	enum MessageHeader{
		CONNECTION_REQUEST 					= 0x0, //[2] uint16_t (timeout in milliseconds)
		CONNECTION_CONFIRMATION 			= 0x1, //[2] uint16_t (timeout in milliseconds)
		HEARTBEAT 							= 0x2, //[0]
		IDENTIFICATION_REQUEST 				= 0x3, //[0]
		IDENTIFICATION_REPLY 				= 0x4, //[x] string (device name)
		DEVICE_ENUMERATION_REQUEST 			= 0x5, //[0]
		DEVICE_ENUMERATION_REPLY 			= 0x6, //[deviceCount] uint8_t (deviceTypeCode for each iodevice)
		ALL_INPUT_DEVICES_STATE_REQUEST 	= 0x7  //[0]
	};

	enum class ConnectionState{
		NOT_CONNECTED,
		CONNECTION_REQUESTED,
		CONNECTION_CONFIRMATION_RECEIVED,
		IDENTIFICATION_REQUESTED,
		IDENTIFICATION_RECEIVED,
		DEVICE_ENUMERATION_REQUESTED,
		DEVICE_ENUMERATION_RECEIVED,
		CONNECTED
	};

};
