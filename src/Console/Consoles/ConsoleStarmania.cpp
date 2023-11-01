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
	auto devices = console->getIODevices();
	
	std::vector<std::shared_ptr<IODevice>> rgbButtons;
	rgbButtons.insert(rgbButtons.end(), devices.begin(), devices.begin() + 8);
	
    left_rgb_button = devices[0]->toLED_RGB_Button();
    right_rgb_button = devices[7]->toLED_RGB_Button();
    
    goleft_rgb_Button = devices[2]->toLED_RGB_Button();
    stop_rgb_Button = devices[3]->toLED_RGB_Button();
    play_rgb_Button = devices[4]->toLED_RGB_Button();
    goright_rgb_Button = devices[5]->toLED_RGB_Button();
	
    left_selection_button = devices[16]->toPushButton();
    right_selection_button = devices[17]->toPushButton();
    rearm_button = devices[18]->toPushButton();
    
	joystickLeft = devices[19]->toJoystick2X();
	joystickRight = devices[20]->toJoystick2X();
	joystickButtonLeft = devices[1]->toLED_RGB_Button();
	joystickButtonRight = devices[6]->toLED_RGB_Button();
	
    square_rgb_button_1 = devices[11]->toLED_RGB_Button();
    square_rgb_button_2 = devices[15]->toLED_RGB_Button();
    square_rgb_button_3 = devices[10]->toLED_RGB_Button();
    square_rgb_button_4 = devices[14]->toLED_RGB_Button();
    square_rgb_button_5 = devices[9]->toLED_RGB_Button();
    square_rgb_button_6 = devices[13]->toLED_RGB_Button();
    square_rgb_button_7 = devices[8]->toLED_RGB_Button();
    square_rgb_button_8 = devices[12]->toLED_RGB_Button();
	
    left_rgb_button->setInputUpdateCallback([&](){
		if(!left_rgb_button->isPressed()){
			if(Environnement::isRunning()) Environnement::stop();
			else Environnement::start();
		}
	});
	
    left_rgb_button->setOutputUpdateCallback([&](){
		if(Environnement::isRunning()) left_rgb_button->setColor(glm::vec3(0.f, 1.f, 0.f));
		else if(Environnement::isStarting()) left_rgb_button->setColor(glm::vec3(1.f, 1.f, 0.f));
		else left_rgb_button->setColor(glm::vec3(0.f, 0.f, 1.f));
	});
    
    
	
    
    
    
    
    
    //TODO: assign these to machines
	joystickLeft->setInputUpdateCallback([&](){
		auto joystick = joystickLeft->toJoystick2X();
		auto position = joystick->getPosition();
		//Logger::warn("Joystick Left: {} {}", position.x, position.y);
	});
	joystickRight->setInputUpdateCallback([&](){
        auto joystick = joystickRight->toJoystick2X();
        auto position = joystick->getPosition();
        //Logger::warn("Joystick Right: {} {}", position.x, position.y);
    });
	
    
    
    
    
    
    left_selection_button->setInputUpdateCallback([&](){
        if(left_selection_button->isPressed()) Stacato::Editor::getCurrentProject()->getCurrentPlot()->getManoeuvreList()->selectPreviousManoeuvre();
    });
    
    right_selection_button->setInputUpdateCallback([&](){
        if(right_selection_button->isPressed()) Stacato::Editor::getCurrentProject()->getCurrentPlot()->getManoeuvreList()->selectNextManoeuvre();
    });
	
    rearm_button->setInputUpdateCallback([&](){
        if(!rearm_button->isPressed()) Environnement::enableAllMachines();
    });
    
    
    
    right_rgb_button->setInputUpdateCallback([&](){
        if(!right_rgb_button->isPressed()) Environnement::disableAllMachines();
    });
    
    right_rgb_button->setOutputUpdateCallback([&](){
        if(Environnement::areAllMachinesEnabled()) right_rgb_button->setColor(glm::vec3(.0f, 1.f, .0f));
        else if(Environnement::areNoMachinesEnabled()) right_rgb_button->setColor(glm::vec3(.0f, .0f, 1.f));
        else right_rgb_button->setColor(glm::vec3(1.f, 1.f, .0f));
    });
    
    
    
    
    goleft_rgb_Button->setInputUpdateCallback([&](){
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(!manoeuvre || !manoeuvre->canRapidToStart()) return;
        if(!goleft_rgb_Button->isPressed()) manoeuvre->rapidToStart();
    });
    
    goleft_rgb_Button->setOutputUpdateCallback([&](){
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(manoeuvre == nullptr) goleft_rgb_Button->setColor(glm::vec3(0.f, 0.f, 0.f));
        else if(manoeuvre->isAtStart()) goleft_rgb_Button->setColor(glm::vec3(.0f, 1.f, .0f));
        else if(manoeuvre->canRapidToStart()) goleft_rgb_Button->setColor(glm::vec3(.1f, .1f, .1f));
        else goleft_rgb_Button->setColor(glm::vec3(.0f, .0f, .1f));
    });
    
    
	
    
    goright_rgb_Button->setInputUpdateCallback([&](){
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(!manoeuvre || !manoeuvre->canRapidToTarget()) return;
        if(!goright_rgb_Button->isPressed()) manoeuvre->rapidToTarget();
    });
    
    goright_rgb_Button->setOutputUpdateCallback([&](){
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(manoeuvre == nullptr) goright_rgb_Button->setColor(glm::vec3(0.f, 0.f, 0.f));
        else if(manoeuvre->isAtTarget()) goright_rgb_Button->setColor(glm::vec3(.0f, 1.f, .0f));
        else if(manoeuvre->canRapidToTarget()) goright_rgb_Button->setColor(glm::vec3(.1f, .1f, .1f));
        else goright_rgb_Button->setColor(glm::vec3(.0f, .0f, .1f));
    });
    
    
    
    stop_rgb_Button->setInputUpdateCallback([&](){
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(stop_rgb_Button->isPressed()){
            if(manoeuvre && manoeuvre->canStop()) {
                manoeuvre->stop();
            }
            else if(PlaybackManager::isAnyAnimationActive()) PlaybackManager::stopAllAnimations();
        }
    });
    
	
    stop_rgb_Button->setOutputUpdateCallback([&](){
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(manoeuvre != nullptr && manoeuvre->canStop()){
            glm::vec3 black = glm::vec3(0.f, 0.f, 0.f);
            glm::vec3 red = glm::vec3(1.f, 0.f, 0.f);
            float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
            stop_rgb_Button->setColor(lerpColor(black, red, lerp));
        }else if(PlaybackManager::isAnyAnimationActive()) stop_rgb_Button->setColor(glm::vec3(1.f, .0f, .0f));
        else stop_rgb_Button->setColor(glm::vec3(.0f, .0f, .1f));
    });
	 
    
    
    
    play_rgb_Button->setInputUpdateCallback([&](){
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(!manoeuvre) return;
        if(!play_rgb_Button->isPressed()) {
            if(manoeuvre->canPausePlayback()) manoeuvre->pausePlayback();
            else if(manoeuvre->canStartPlayback()) manoeuvre->startPlayback();
            else if(manoeuvre->canRapidToPlaybackPosition()) manoeuvre->rapidToPlaybackPosition();
        }
    });
    
	
    play_rgb_Button->setOutputUpdateCallback([&](){
        auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(manoeuvre == nullptr) play_rgb_Button->setColor(glm::vec3(0.f, 0.f, 0.1f));
        else if(manoeuvre->isPlaying()){
            glm::vec3 black = glm::vec3(0.f);
            glm::vec3 green = glm::vec3(0.f, 1.f, 0.f);
            float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
            play_rgb_Button->setColor(lerpColor(black, green, lerp));
        }else if(manoeuvre->isInRapid()){
            glm::vec3 white = glm::vec3(1.f);
            glm::vec3 yellow = glm::vec3(1.f, .7f, 0.f);
            float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
            play_rgb_Button->setColor(lerpColor(white, yellow, lerp));
        }
        else if(manoeuvre->canStartPlayback()) play_rgb_Button->setColor(glm::vec3(0.1f, 0.1f, 0.1f));
        else if(manoeuvre->canRapidToPlaybackPosition()) play_rgb_Button->setColor(glm::vec3(1.f, .7f, 0.f));
        else play_rgb_Button->setColor(glm::vec3(.0f, .0f, .1f));
    });
    
	
}



void ConsoleStarmania::onDisconnection(){
	
}
