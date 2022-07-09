#include <pch.h>
#include "StacatoCompact.h"
#include "Console/Console.h"

#include "Environnement/Environnement.h"

void StacatoCompactMapping::apply(std::shared_ptr<Console> console){
	auto devices = console->getIODevices();
	
	std::vector<std::shared_ptr<IODevice>> rgbButtons;
	rgbButtons.insert(rgbButtons.end(), devices.begin(), devices.begin() + 8);
	
	auto rgb_Button_0 = devices[0];
	auto rgb_Button_1 = devices[1];
	auto rgb_Button_2 = devices[2];
	auto rgb_Button_3 = devices[3];
	auto rgb_Button_4 = devices[4];
	auto rgb_Button_5 = devices[5];
	auto rgb_Button_6 = devices[6];
	auto rgb_Button_7 = devices[7];
	
	
	auto pushButton_0 = devices[8];
	auto pushButton_1 = devices[9];
	auto pushButton_2 = devices[10];
	
	auto bluePWM_LED = devices[11];
	auto greenPWM_LED_1 = devices[12];
	auto greenPWM_LED_2 = devices[13];
	auto redPWM_LED = devices[14];
	
	auto joystick = devices[15];
	
	rgb_Button_0->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
		auto button = device->toPushButton();
		if(!button->isPressed()){
			if(Environnement::isRunning()) Environnement::stop();
			else Environnement::start();
		}
	});
	
	rgb_Button_0->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
		auto rgbLed = device->toLED_RGB();
		if(Environnement::isRunning()) rgbLed->setColor(glm::vec3(0.f, 1.f, 0.f));
		else if(Environnement::isStarting()) rgbLed->setColor(glm::vec3(1.f, 1.f, 0.f));
		else rgbLed->setColor(glm::vec3(0.f, 0.f, 1.f));
	});
	
	joystick->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
		auto joystick = device->toJoystick2X();
		auto position = joystick->getPosition();
		Logger::warn("Joystick: {} {}", position.x, position.y);
	});
	
	
	
}
