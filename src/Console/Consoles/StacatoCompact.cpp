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
		
	pwmLed1 = devices[0]->toLED_PWM();
	pwmLed2 = devices[1]->toLED_PWM();
	pwmLed3 = devices[2]->toLED_PWM();
	pwmLed4 = devices[3]->toLED_PWM();
	rgbButton1 = devices[4]->toLED_RGB_Button();
	rgbButton2 = devices[5]->toLED_RGB_Button();
	rgbButton3 = devices[6]->toLED_RGB_Button();
	rgbButton4 = devices[7]->toLED_RGB_Button();
	rgbButton5 = devices[8]->toLED_RGB_Button();
	rgbButton6 = devices[9]->toLED_RGB_Button();
	rgbButton7 = devices[10]->toLED_RGB_Button();
	rgbButton8 = devices[11]->toLED_RGB_Button();
	pushButton1 = devices[12]->toPushButton();
	pushButton2 = devices[13]->toPushButton();
	pushButton3 = devices[14]->toPushButton();
	joystick = devices[15]->toJoystick2X();
	
	pwmLed1->setOutputUpdateCallback([&](){ pwmLed1->setBrightness(fmod(Timing::getProgramTime_seconds() + 0.00, 1.0)); });
	pwmLed2->setOutputUpdateCallback([&](){ pwmLed2->setBrightness(fmod(Timing::getProgramTime_seconds() + 0.25, 1.0)); });
	pwmLed3->setOutputUpdateCallback([&](){ pwmLed3->setBrightness(fmod(Timing::getProgramTime_seconds() + 0.50, 1.0)); });
	pwmLed4->setOutputUpdateCallback([&](){ pwmLed4->setBrightness(fmod(Timing::getProgramTime_seconds() + 0.75, 1.0)); });
	
	rgbButton1->setInputUpdateCallback([&](){ Logger::warn("RGB Button 1 : {}", rgbButton1->isPressed()); });
	rgbButton2->setInputUpdateCallback([&](){ Logger::warn("RGB Button 2 : {}", rgbButton2->isPressed()); });
	rgbButton3->setInputUpdateCallback([&](){ Logger::warn("RGB Button 3 : {}", rgbButton3->isPressed()); });
	rgbButton4->setInputUpdateCallback([&](){ Logger::warn("RGB Button 4 : {}", rgbButton4->isPressed()); });
	rgbButton5->setInputUpdateCallback([&](){ Logger::warn("RGB Button 5 : {}", rgbButton5->isPressed()); });
	rgbButton6->setInputUpdateCallback([&](){ Logger::warn("RGB Button 6 : {}", rgbButton6->isPressed()); });
	rgbButton7->setInputUpdateCallback([&](){ Logger::warn("RGB Button 7 : {}", rgbButton7->isPressed()); });
	rgbButton8->setInputUpdateCallback([&](){ Logger::warn("RGB Button 8 : {}", rgbButton8->isPressed()); });
	
	pushButton1->setInputUpdateCallback([&](){ Logger::warn("Button 1 : {}", pushButton1->isPressed()); });
	pushButton2->setInputUpdateCallback([&](){ Logger::warn("Button 2 : {}", pushButton2->isPressed()); });
	pushButton3->setInputUpdateCallback([&](){ Logger::warn("Button 3 : {}", pushButton3->isPressed()); });
	pushButton4->setInputUpdateCallback([&](){ Logger::warn("Button 4 : {}", pushButton4->isPressed()); });
	pushButton5->setInputUpdateCallback([&](){ Logger::warn("Button 5 : {}", pushButton5->isPressed()); });
	
	joystick->setInputUpdateCallback([&](){
		Environnement::getManualControlChannel()->setControlValue(joystick->getPosition().y);
	});
	
	/*
    left_rgb_button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
		auto button = device->toPushButton();
		if(!button->isPressed()){
			if(Environnement::isRunning()) Environnement::stop();
			else Environnement::start();
		}
	});
	
    left_rgb_button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
		auto rgbLed = device->toLED_RGB();
		if(Environnement::isRunning()) rgbLed->setColor(glm::vec3(0.f, 1.f, 0.f));
		else if(Environnement::isStarting()) rgbLed->setColor(glm::vec3(1.f, 1.f, 0.f));
		else rgbLed->setColor(glm::vec3(0.f, 0.f, 1.f));
	});
    
    left_selection_button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        if(button->isPressed()) Stacato::Editor::getCurrentProject()->getCurrentPlot()->getManoeuvreList()->selectPreviousManoeuvre();
    });
    
    right_selection_button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        if(button->isPressed()) Stacato::Editor::getCurrentProject()->getCurrentPlot()->getManoeuvreList()->selectNextManoeuvre();
    });
	
    rearm_button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        if(!button->isPressed()) Environnement::enableAllMachines();
    });
    
    
    
    right_rgb_button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        if(!button->isPressed()) Environnement::disableAllMachines();
    });
    
    right_rgb_button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto rgbLed = device->toLED_RGB();
        if(Environnement::areAllMachinesEnabled()) rgbLed->setColor(glm::vec3(.0f, 1.f, .0f));
        else if(Environnement::areNoMachinesEnabled()) rgbLed->setColor(glm::vec3(.0f, .0f, 1.f));
        else rgbLed->setColor(glm::vec3(1.f, 1.f, .0f));
    });
    
    
    
    
    goleft_rgb_Button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(!manoeuvre || !manoeuvre->canRapidToStart()) return;
        if(!button->isPressed()) manoeuvre->rapidToStart();
    });
    
    goleft_rgb_Button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto rgbLed = device->toLED_RGB();
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(manoeuvre == nullptr) rgbLed->setColor(glm::vec3(0.f, 0.f, 0.f));
        else if(manoeuvre->isAtStart()) rgbLed->setColor(glm::vec3(.0f, 1.f, .0f));
        else if(manoeuvre->canRapidToStart()) rgbLed->setColor(glm::vec3(.1f, .1f, .1f));
        else rgbLed->setColor(glm::vec3(.0f, .0f, .1f));
    });
    
    
	
    
    goright_rgb_Button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(!manoeuvre || !manoeuvre->canRapidToTarget()) return;
        if(!button->isPressed()) manoeuvre->rapidToTarget();
    });
    
    goright_rgb_Button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto rgbLed = device->toLED_RGB();
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(manoeuvre == nullptr) rgbLed->setColor(glm::vec3(0.f, 0.f, 0.f));
        else if(manoeuvre->isAtTarget()) rgbLed->setColor(glm::vec3(.0f, 1.f, .0f));
        else if(manoeuvre->canRapidToTarget()) rgbLed->setColor(glm::vec3(.1f, .1f, .1f));
        else rgbLed->setColor(glm::vec3(.0f, .0f, .1f));
    });
    
    
    
    stop_rgb_Button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(button->isPressed()){
            if(manoeuvre && manoeuvre->canStop()) {
                manoeuvre->stop();
            }
            else if(PlaybackManager::isAnyAnimationActive()) PlaybackManager::stopAllAnimations();
        }
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
	 
    
    
    
    play_rgb_Button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(!manoeuvre) return;
        if(!button->isPressed()) {
            if(manoeuvre->canPausePlayback()) manoeuvre->pausePlayback();
            else if(manoeuvre->canStartPlayback()) manoeuvre->startPlayback();
            else if(manoeuvre->canRapidToPlaybackPosition()) manoeuvre->rapidToPlaybackPosition();
        }
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
