#include <pch.h>
#include "StacatoCompact.h"
#include "Console/Console.h"

#include "Environnement/Environnement.h"
#include "Plot/Plot.h"
#include "Plot/ManoeuvreList.h"
#include "Animation/Manoeuvre.h"

#include "Stacato/StacatoEditor.h"
#include "Stacato/Project/StacatoProject.h"

#include "Animation/ManualControlChannel.h"

void StacatoCompact::apply(std::shared_ptr<Console> console){
	auto devices = console->getIODevices();
	
	pwmLed1 = devices[0]->toLED_PWM(); 				//V1 console connection
	pwmLed2 = devices[1]->toLED_PWM(); 				//V2 environnement status
	pwmLed3 = devices[2]->toLED_PWM(); 				//V3 machine status
	rgbButton1 = devices[3]->toLED_RGB_Button();	//BP4 ??
	rgbButton2 = devices[4]->toLED_RGB_Button();	//BP5 ??
	rgbButton3 = devices[5]->toLED_RGB_Button();	//BP6 ??
	rgbButton4 = devices[6]->toLED_RGB_Button();	//BP7 ??
	rgbButton5 = devices[7]->toLED_RGB_Button();	//BP8 rapid to start
	rgbButton6 = devices[8]->toLED_RGB_Button();	//BP9 Stop/Pause
	rgbButton7 = devices[9]->toLED_RGB_Button();	//BP10 Play
	rgbButton8 = devices[10]->toLED_RGB_Button();	//BP11 rapid to end
	pushButton1 = devices[11]->toPushButton();		//BP1 start/stop environnement
	pushButton2 = devices[12]->toPushButton();		//BP2 enable all machines
	pushButton3 = devices[13]->toPushButton();		//BP3 disable all machines
	pushButton4 = devices[14]->toPushButton();		//BP12 manoeuvre list selection up
	pushButton5 = devices[15]->toPushButton();		//BP13 manoeuvre list selection down
	joystick = devices[16]->toJoystick2X();
	
	pwmLed1->setOutputUpdateCallback([&](){
		pwmLed1->setBrightness(Timing::getSinusWave(1.0, 0.0, 1.0));
	});
	pwmLed2->setOutputUpdateCallback([&](){
		if(Environnement::isRunning()) pwmLed2->setBrightness(1.0);
		else if(Environnement::isStarting()) pwmLed2->setBrightness(Timing::getTriangleWave(.3, 0, 1));
		else pwmLed2->setBrightness(0.0);
	});
	pwmLed3->setOutputUpdateCallback([&](){
		if(Environnement::areAllMachinesEnabled()) pwmLed3->setBrightness(1.0);
		else if(Environnement::areNoMachinesEnabled()) pwmLed3->setBrightness(0.0);
		else pwmLed3->setBrightness(Timing::getBlink(.5) ? 1.0 : 0.0);
	});
	
	
	rgbButton1->setInputUpdateCallback([&](){ Logger::trace("RGB Button 1 : {}", rgbButton1->isPressed()); });
	rgbButton2->setInputUpdateCallback([&](){ Logger::trace("RGB Button 2 : {}", rgbButton2->isPressed()); });
	rgbButton3->setInputUpdateCallback([&](){ Logger::trace("RGB Button 3 : {}", rgbButton3->isPressed()); });
	rgbButton4->setInputUpdateCallback([&](){ Logger::trace("RGB Button 4 : {}", rgbButton4->isPressed()); });
	rgbButton5->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 5 : {}", rgbButton5->isPressed());
		auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
		if(!manoeuvre || !manoeuvre->canRapidToStart()) return;
		if(!rgbButton5->isPressed()) manoeuvre->rapidToStart();
	});
	rgbButton6->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 6 : {}", rgbButton6->isPressed());
		auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
		if(rgbButton6->isPressed()){
			if(manoeuvre && manoeuvre->canStop()) {
				manoeuvre->stop();
			}
			else if(PlaybackManager::isAnyAnimationActive()) PlaybackManager::stopAllAnimations();
		}
	});
	rgbButton7->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 7 : {}", rgbButton7->isPressed());
		auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
		if(!manoeuvre) return;
		if(!rgbButton7->isPressed()) {
			if(manoeuvre->canPausePlayback()) manoeuvre->pausePlayback();
			else if(manoeuvre->canStartPlayback()) manoeuvre->startPlayback();
			else if(manoeuvre->canRapidToPlaybackPosition()) manoeuvre->rapidToPlaybackPosition();
		}
	});
	rgbButton8->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 8 : {}", rgbButton8->isPressed());
		auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
		if(!manoeuvre || !manoeuvre->canRapidToTarget()) return;
		if(!rgbButton8->isPressed()) manoeuvre->rapidToTarget();
	});
	
	
	
	pushButton1->setInputUpdateCallback([&](){
		Logger::trace("Button 1 : {}", pushButton1->isPressed());
		if(!pushButton1->isPressed()){
			if(Environnement::isRunning()) Environnement::stop();
			else Environnement::start();
		}
	});
	pushButton2->setInputUpdateCallback([&](){
		Logger::trace("Button 2 : {}", pushButton2->isPressed());
		if(!pushButton2->isPressed()) Environnement::enableAllMachines();
	});
	pushButton3->setInputUpdateCallback([&](){
		Logger::trace("Button 3 : {}", pushButton3->isPressed());
		if(!pushButton3->isPressed()) Environnement::disableAllMachines();
	});
	pushButton4->setInputUpdateCallback([&](){
		Logger::trace("Button 4 : {}", pushButton4->isPressed());
		if(pushButton4->isPressed()) Stacato::Editor::getCurrentProject()->getCurrentPlot()->getManoeuvreList()->selectPreviousManoeuvre();
	});
	pushButton5->setInputUpdateCallback([&](){
		Logger::trace("Button 5 : {}", pushButton5->isPressed());
		if(pushButton5->isPressed()) Stacato::Editor::getCurrentProject()->getCurrentPlot()->getManoeuvreList()->selectNextManoeuvre();
	});
	
	joystick->setInputUpdateCallback([&](){
		Environnement::getManualControlChannel()->setControlValue(joystick->getPosition().y);
	});
	
	/*
    goleft_rgb_Button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto rgbLed = device->toLED_RGB();
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(manoeuvre == nullptr) rgbLed->setColor(glm::vec3(0.f, 0.f, 0.f));
        else if(manoeuvre->isAtStart()) rgbLed->setColor(glm::vec3(.0f, 1.f, .0f));
        else if(manoeuvre->canRapidToStart()) rgbLed->setColor(glm::vec3(.1f, .1f, .1f));
        else rgbLed->setColor(glm::vec3(.0f, .0f, .1f));
    });
    
    goright_rgb_Button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto rgbLed = device->toLED_RGB();
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(manoeuvre == nullptr) rgbLed->setColor(glm::vec3(0.f, 0.f, 0.f));
        else if(manoeuvre->isAtTarget()) rgbLed->setColor(glm::vec3(.0f, 1.f, .0f));
        else if(manoeuvre->canRapidToTarget()) rgbLed->setColor(glm::vec3(.1f, .1f, .1f));
        else rgbLed->setColor(glm::vec3(.0f, .0f, .1f));
    });
	
    stop_rgb_Button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto rgbLed = device->toLED_RGB();
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(manoeuvre != nullptr && manoeuvre->canStop()){
            glm::vec3 black = glm::vec3(0.f, 0.f, 0.f);
            glm::vec3 red = glm::vec3(1.f, 0.f, 0.f);
            float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
			rgbLed->setColor(lerpColor(black, red, lerp));
        }else if(PlaybackManager::isAnyAnimationActive()) rgbLed->setColor(glm::vec3(1.f, .0f, .0f));
        else rgbLed->setColor(glm::vec3(.0f, .0f, .1f));
    });
	 
    play_rgb_Button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto rgbLed = device->toLED_RGB();
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(manoeuvre == nullptr) rgbLed->setColor(glm::vec3(0.f, 0.f, 0.1f));
        else if(manoeuvre->isPlaying()){
            glm::vec3 black = glm::vec3(0.f);
            glm::vec3 green = glm::vec3(0.f, 1.f, 0.f);
            float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
			rgbLed->setColor(lerpColor(black, green, lerp));
        }else if(manoeuvre->isInRapid()){
            glm::vec3 white = glm::vec3(1.f);
            glm::vec3 yellow = glm::vec3(1.f, .7f, 0.f);
            float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
            rgbLed->setColor(lerpColor(white, yellow, lerp));
        }
        else if(manoeuvre->canStartPlayback()) rgbLed->setColor(glm::vec3(0.1f, 0.1f, 0.1f));
        else if(manoeuvre->canRapidToPlaybackPosition()) rgbLed->setColor(glm::vec3(1.f, .7f, 0.f));
        else rgbLed->setColor(glm::vec3(.0f, .0f, .1f));
    });
    */
	
}


void StacatoCompact::onDisconnection(){
	Environnement::getManualControlChannel()->setControlValue(0.0);
}
