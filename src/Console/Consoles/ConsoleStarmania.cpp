#include <pch.h>
#include "ConsoleStarmania.h"
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

void ConsoleStarmania::apply(std::shared_ptr<Console> console){
	/*
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
    
	joystickLeft = devices[19]->toJoystick2X();
	joystickRight = devices[20]->toJoystick2X();
	joystickButtonLeft = devices[1]->toLED_RGB_Button();
	joystickButtonRight = devices[6]->toLED_RGB_Button();
	
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
    
    
	
    
    
    
    
    
    //TODO: assign these to machines
	joystickLeft->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
		auto joystick = device->toJoystick2X();
		auto position = joystick->getPosition();
		//Logger::warn("Joystick Left: {} {}", position.x, position.y);
	});
	joystickRight->setInputUpdateCallback([](std::shared_ptr<IODevice> device){
        auto joystick = device->toJoystick2X();
        auto position = joystick->getPosition();
        //Logger::warn("Joystick Right: {} {}", position.x, position.y);
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
