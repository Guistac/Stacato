#include <pch.h>
#include "ButtonBoard.h"

#include "Console/Console.h"

void ButtonBoardMapping::apply(std::shared_ptr<Console> console){
	auto devices = console->getIODevices();
	
	auto rgb_Button_1 = std::dynamic_pointer_cast<LED_RGB_Button>(devices.front());
	
	rgb_Button_1->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
		auto rgbButton = std::dynamic_pointer_cast<LED_RGB_Button>(device);
		if(rgbButton->isPressed()){
			Logger::warn("RGB 1 Pressed");
		}else{
			Logger::warn("RGB 1 Released");
		}
	});
	
	rgb_Button_1->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
		auto rgbButton = std::dynamic_pointer_cast<LED_RGB_Button>(device);
		if(fmod(Timing::getProgramTime_seconds(), .5f) > .25f){
			rgbButton->setColor(glm::vec3(1.f, 0.f, 0.f));
		}else{
			rgbButton->setColor(glm::vec3(0.f, 0.f, 1.f));
		}
	});
	
	
	
}
