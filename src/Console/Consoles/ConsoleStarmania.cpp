#include <pch.h>
#include "ConsoleStarmania.h"
#include "Console/Console.h"

#include "Project/Project.h"
#include "Environnement/Environnement.h"
#include "Plot/Plot.h"
#include "Plot/ManoeuvreList.h"
#include "Animation/Manoeuvre.h"

void ConsoleStarmaniaMapping::apply(std::shared_ptr<Console> console){
	auto devices = console->getIODevices();
	
	std::vector<std::shared_ptr<IODevice>> rgbButtons;
	rgbButtons.insert(rgbButtons.end(), devices.begin(), devices.begin() + 8);
	
    auto left_rgb_button = devices[0];
    auto right_rgb_button = devices[7];
    
	auto goleft_rgb_Button = devices[2];
    auto stop_rgb_Button = devices[3];
    auto play_rgb_Button = devices[4];
	auto goright_rgb_Button = devices[5];
	
	auto left_selection_button = devices[16];
	auto right_selection_button = devices[17];
	auto rearm_button = devices[18];
	
	auto left_joystick = devices[20];
    auto right_joystick = devices[19];
    auto left_joystick_rgb_button = devices[1];
    auto right_joystick_rgb_button = devices[6];
    
    auto square_rgb_button_1 = devices[11];
    auto square_rgb_button_2 = devices[15];
    auto square_rgb_button_3 = devices[10];
    auto square_rgb_button_4 = devices[14];
    auto square_rgb_button_5 = devices[9];
    auto square_rgb_button_6 = devices[13];
    auto square_rgb_button_7 = devices[8];
    auto square_rgb_button_8 = devices[12];
    
    
	
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
    
    
	
    
    
    
    
    
    
    left_joystick->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
		auto joystick = device->toJoystick2X();
		auto position = joystick->getPosition();
		Logger::warn("Joystick Left: {} {}", position.x, position.y);
	});
    right_joystick->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto joystick = device->toJoystick2X();
        auto position = joystick->getPosition();
        Logger::warn("Joystick Right: {} {}", position.x, position.y);
    });
	
    
    
    
    
    
    left_selection_button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        if(button->isPressed()) Project::currentPlot->getManoeuvreList()->selectPreviousManoeuvre();
    });
    
    right_selection_button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        if(button->isPressed()) Project::currentPlot->getManoeuvreList()->selectNextManoeuvre();
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
        auto manoeuvre = Project::currentPlot->getSelectedManoeuvre();
        if(!manoeuvre || !manoeuvre->canRapidToStart()) return;
        if(!button->isPressed()) manoeuvre->rapidToStart();
    });
    
    goleft_rgb_Button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto rgbLed = device->toLED_RGB();
        auto manoeuvre = Project::currentPlot->getSelectedManoeuvre();
        if(manoeuvre == nullptr) rgbLed->setColor(glm::vec3(0.f, 0.f, 0.f));
        else if(manoeuvre->isAtStart()) rgbLed->setColor(glm::vec3(.0f, 1.f, .0f));
        else rgbLed->setColor(glm::vec3(.0f, .0f, .0f));
    });
    
    
	
    
    goright_rgb_Button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        auto manoeuvre = Project::currentPlot->getSelectedManoeuvre();
        if(!manoeuvre || !manoeuvre->canRapidToTarget()) return;
        if(!button->isPressed()) manoeuvre->rapidToTarget();
    });
    
    goright_rgb_Button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto rgbLed = device->toLED_RGB();
        auto manoeuvre = Project::currentPlot->getSelectedManoeuvre();
        if(manoeuvre == nullptr) rgbLed->setColor(glm::vec3(0.f, 0.f, 0.f));
        else if(manoeuvre->isAtTarget()) rgbLed->setColor(glm::vec3(.0f, 1.f, .0f));
        else rgbLed->setColor(glm::vec3(.0f, .0f, .0f));
    });
    
    
    
    stop_rgb_Button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        auto manoeuvre = Project::currentPlot->getSelectedManoeuvre();
        if(manoeuvre || manoeuvre->canStop()) manoeuvre->stop();
        else if(PlaybackManager::isAnyAnimationActive()) PlaybackManager::stopAllAnimations();
    });
    
	
    stop_rgb_Button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto rgbLed = device->toLED_RGB();
        auto manoeuvre = Project::currentPlot->getSelectedManoeuvre();
        if(manoeuvre != nullptr && manoeuvre->canStop()){
            glm::vec3 yellow = glm::vec3(1.f, 1.f, 0.f);
            glm::vec3 red = glm::vec3(1.f, 0.f, 0.f);
            float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
            //rgbLed->setColor( glm::lerp(yellow, red, lerp));
        }else if(PlaybackManager::isAnyAnimationActive()) rgbLed->setColor(glm::vec3(1.f, .0f, .0f));
        else rgbLed->setColor(glm::vec3(.0f));
		 
    });
	 
    
    
    
    play_rgb_Button->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto button = device->toPushButton();
        auto manoeuvre = Project::currentPlot->getSelectedManoeuvre();
        if(!manoeuvre || !manoeuvre->canRapidToStart()) return;
        if(!button->isPressed()) {
            if(manoeuvre->canPausePlayback()) manoeuvre->pausePlayback();
            else if(manoeuvre->canStartPlayback()) manoeuvre->startPlayback();
            else if(manoeuvre->canRapidToPlaybackPosition()) manoeuvre->rapidToPlaybackPosition();
        }
    });
    
	
    play_rgb_Button->setOutputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto rgbLed = device->toLED_RGB();
        auto manoeuvre = Project::currentPlot->getSelectedManoeuvre();
        if(manoeuvre == nullptr) rgbLed->setColor(glm::vec3(0.f, 0.f, 0.f));
        else if(manoeuvre->canPausePlayback()){
            glm::vec3 white = glm::vec3(1.f, 1.f, 1.f);
            glm::vec3 green = glm::vec3(0.f, 1.f, 0.f);
            float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
            //rgbLed->setColor( glm::lerp(white, green, lerp));
        }else if(manoeuvre->canStartPlayback()) rgbLed->setColor(glm::vec3(0.f, 1.f, 0.f));
        else rgbLed->setColor(glm::vec3(0.f));
    });
    
	
}
