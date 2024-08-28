#include "StacatoV2.h"

#include <pch.h>
#include "Console/Console.h"

#include "Environnement/Environnement.h"
#include "Plot/Plot.h"
#include "Plot/ManoeuvreList.h"
#include "Animation/Manoeuvre.h"

#include "Stacato/StacatoEditor.h"
#include "Stacato/Project/StacatoProject.h"

//TODO: using glm::vec3 arithmetic such as end - start causes the stage visualize to malfunction. figure out why this happens !
static glm::vec3 lerpColor(glm::vec3 start, glm::vec3 end, float progress){
	return glm::vec3(start.x + (end.x - start.x) * progress,
					 start.y + (end.y - start.y) * progress,
					 start.z + (end.z - start.z) * progress);
};

void StacatoV2::apply(std::shared_ptr<Console> console){
	auto devices = console->getIODevices();
	
	led_EtherCat = 						devices[0]->toLED_PWM();
	led_Light = 						devices[1]->toLED_PWM();
	led_Video = 						devices[2]->toLED_PWM();
	rgbButton_goToStart = 				devices[3]->toLED_RGB_Button();
	rgbButton_PlayPause = 				devices[4]->toLED_RGB_Button();
	rgbButton_Stop = 					devices[5]->toLED_RGB_Button();
	rgbButton_goToEnd = 				devices[6]->toLED_RGB_Button();
	pushbutton_arrowUp = 				devices[7]->toPushButton();
	pushbutton_arrowDown = 				devices[8]->toPushButton();
	joystick3x_left = 					devices[9]->toJoystick3X();
	joystick3x_right = 					devices[10]->toJoystick3X();
	pushbutton_leftJoystickDeadman = 	devices[11]->toPushButton();
	pushbutton_rightJoystickDeadman = 	devices[12]->toPushButton();
	pushbutton_leftJoystickSelection = 	devices[13]->toPushButton();
	pushbutton_rightJoystickSelection =	devices[14]->toPushButton();
	
	
	//======= STATUS LEDS ========
	
	led_EtherCat->setOutputUpdateCallback([&](){
		float br = Timing::getSinusWave(1.0, 0.0, 1.0);
		led_EtherCat->setBrightness(br);
	});
	led_Light->setOutputUpdateCallback([&](){
		float br = Timing::getSinusWave(1.0, 0.0, 1.0);
		led_Light->setBrightness(br);
	});
	led_Video->setOutputUpdateCallback([&](){
		float br = Timing::getSinusWave(1.0, 0.0, 1.0);
		led_Video->setBrightness(br);
	});

	
	
	//======= RGB LED BUTTONS ========
	
	rgbButton_goToStart->setInputUpdateCallback([](){});
	rgbButton_goToStart->setOutputUpdateCallback([&](){
		float br = Timing::getSinusWave(1.0, 0.0, 1.0);
		rgbButton_goToStart->setColor(glm::vec3(br,br,br));
	});
	
	rgbButton_PlayPause->setInputUpdateCallback([](){});
	rgbButton_PlayPause->setOutputUpdateCallback([&](){
		float br = Timing::getSinusWave(1.0, 0.0, 1.0);
		rgbButton_PlayPause->setColor(glm::vec3(br,br,br));
	});
	
	rgbButton_Stop->setInputUpdateCallback([](){});
	rgbButton_Stop->setOutputUpdateCallback([&](){
		float br = Timing::getSinusWave(1.0, 0.0, 1.0);
		rgbButton_Stop->setColor(glm::vec3(br,br,br));
	});
	
	rgbButton_goToEnd->setInputUpdateCallback([](){});
	rgbButton_goToEnd->setOutputUpdateCallback([&](){
		float br = Timing::getSinusWave(1.0, 0.0, 1.0);
		rgbButton_goToEnd->setColor(glm::vec3(br,br,br));
	});
	
	
	
	//======= SCROLL BUTTONS ========
	
	pushbutton_arrowUp->setInputUpdateCallback([](){});
	pushbutton_arrowDown->setInputUpdateCallback([](){});
	
	
	//======= JOYSTICKS ========
	
	joystick3x_left->setInputUpdateCallback([&,this](){
		glm::vec3 pos = joystick3x_left->getPosition();
		leftJoystickControlChannel->setControlValue(pos.x, pos.y, pos.z);
	});
	joystick3x_right->setInputUpdateCallback([&](){
		glm::vec3 pos = joystick3x_right->getPosition();
		rightJoystickControlChannel->setControlValue(pos.x, pos.y, pos.z);
	});
	pushbutton_leftJoystickDeadman->setInputUpdateCallback([](){});
	pushbutton_rightJoystickDeadman->setInputUpdateCallback([](){});
	pushbutton_leftJoystickSelection->setInputUpdateCallback([](){});
	pushbutton_rightJoystickSelection->setInputUpdateCallback([](){});
}



void StacatoV2::onDisconnection(){
	leftJoystickControlChannel->setControlValue(0.0, 0.0, 0.0);
	rightJoystickControlChannel->setControlValue(0.0, 0.0, 0.0);
}

