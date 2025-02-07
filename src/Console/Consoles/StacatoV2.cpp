#include "StacatoV2.h"

#include <pch.h>
#include "Console/Console.h"

#include "Environnement/Environnement.h"
#include "Plot/Plot.h"
#include "Plot/ManoeuvreList.h"
#include "Animation/Manoeuvre.h"

#include "Stacato/StacatoEditor.h"
#include "Stacato/Project/StacatoProject.h"
#include "Networking/NetworkDevice.h"

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
	
	led_EtherCat->setOutputUpdateCallback([this](){
		if(Environnement::isRunning()) led_EtherCat->setBrightness(1.0);
		else if(Environnement::isStarting()) led_EtherCat->setBrightness(Timing::getSinusWave(0.2, 0.0, 1.0));
		else led_EtherCat->setBrightness(Timing::getSinusWave(.5, 0.0, 0.05));
	});
	led_Light->setOutputUpdateCallback([this](){
		if(Environnement::getNetworkDevices().size() > 0 && Environnement::getNetworkDevices()[0]->isConnected())
			led_Light->setBrightness(1.0);
		else 
			led_Light->setBrightness(Timing::getSinusWave(.5, 0.0, 0.05));
	});
	led_Video->setOutputUpdateCallback([this](){
		if(Environnement::getNetworkDevices().size() > 1 && Environnement::getNetworkDevices()[1]->isConnected())
			led_Video->setBrightness(1.0);
		else 
			led_Video->setBrightness(Timing::getSinusWave(.5, 0.0, 0.05));
	});

	
	
	//======= RGB LED BUTTONS ========
	
	rgbButton_goToStart->setInputUpdateCallback([this](){
		if(!Stacato::Editor::getCurrentProject()) return;
		auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(!manoeuvre || !manoeuvre->canRapidToStart()) return;
        if(!rgbButton_goToStart->isPressed()) manoeuvre->rapidToStart();
	});
	rgbButton_goToStart->setOutputUpdateCallback([this](){
		auto project = Stacato::Editor::getCurrentProject();
		if(project == nullptr) return;
		auto plot = project->getCurrentPlot();
		if(plot == nullptr) return;
		auto manoeuvre = plot->getSelectedManoeuvre();
        if(manoeuvre == nullptr) rgbButton_goToStart->setColor(glm::vec3(0.f, 0.f, 0.f));
        else if(manoeuvre->isAtStart()) rgbButton_goToStart->setColor(glm::vec3(.0f, 1.f, .0f));
        else if(manoeuvre->canRapidToStart()) rgbButton_goToStart->setColor(glm::vec3(.1f, .1f, .1f));
        else rgbButton_goToStart->setColor(glm::vec3(.0f, .0f, .1f));
	});
	
	rgbButton_PlayPause->setInputUpdateCallback([this](){
		auto project = Stacato::Editor::getCurrentProject();
		if(project == nullptr) return;
		auto plot = project->getCurrentPlot();
		if(plot == nullptr) return;
		auto manoeuvre = plot->getSelectedManoeuvre();
        if(!manoeuvre) return;
        if(!rgbButton_PlayPause->isPressed()) {
            if(manoeuvre->canPausePlayback()) manoeuvre->pausePlayback();
            else if(manoeuvre->canStartPlayback()) manoeuvre->startPlayback();
            else if(manoeuvre->canRapidToPlaybackPosition()) manoeuvre->rapidToPlaybackPosition();
        }
	});
	rgbButton_PlayPause->setOutputUpdateCallback([this](){
        auto project = Stacato::Editor::getCurrentProject();
		if(project == nullptr) return;
		auto plot = project->getCurrentPlot();
		if(plot == nullptr) return;
		auto manoeuvre = plot->getSelectedManoeuvre();
        if(manoeuvre == nullptr) rgbButton_PlayPause->setColor(glm::vec3(0.f, 0.f, 0.1f));
        else if(manoeuvre->isPlaying()){
            glm::vec3 black = glm::vec3(0.f);
            glm::vec3 green = glm::vec3(0.f, 1.f, 0.f);
            float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
            rgbButton_PlayPause->setColor(lerpColor(black, green, lerp));
        }else if(manoeuvre->isInRapid()){
            glm::vec3 white = glm::vec3(1.f);
            glm::vec3 yellow = glm::vec3(1.f, .7f, 0.f);
            float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
            rgbButton_PlayPause->setColor(lerpColor(white, yellow, lerp));
        }
        else if(manoeuvre->canStartPlayback()) rgbButton_PlayPause->setColor(glm::vec3(0.1f, 0.1f, 0.1f));
        else if(manoeuvre->canRapidToPlaybackPosition()) rgbButton_PlayPause->setColor(glm::vec3(1.f, .7f, 0.f));
        else rgbButton_PlayPause->setColor(glm::vec3(.0f, .0f, .1f));
	});
	
	rgbButton_Stop->setInputUpdateCallback([this](){
		auto project = Stacato::Editor::getCurrentProject();
		if(project == nullptr) return;
		auto plot = project->getCurrentPlot();
		if(plot == nullptr) return;
		auto manoeuvre = plot->getSelectedManoeuvre();
        if(rgbButton_Stop->isPressed()){
            if(manoeuvre && manoeuvre->canStop()) {
                manoeuvre->stop();
            }
            else if(PlaybackManager::isAnyAnimationActive()) PlaybackManager::stopAllAnimations();
        }
	});
	rgbButton_Stop->setOutputUpdateCallback([this](){
                auto project = Stacato::Editor::getCurrentProject();
		if(project == nullptr) return;
		auto plot = project->getCurrentPlot();
		if(plot == nullptr) return;
		auto manoeuvre = plot->getSelectedManoeuvre();
        if(manoeuvre != nullptr && manoeuvre->canStop()){
            glm::vec3 black = glm::vec3(0.f, 0.f, 0.f);
            glm::vec3 red = glm::vec3(1.f, 0.f, 0.f);
            float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
            rgbButton_Stop->setColor(lerpColor(black, red, lerp));
        }else if(PlaybackManager::isAnyAnimationActive()) rgbButton_Stop->setColor(glm::vec3(1.f, .0f, .0f));
        else rgbButton_Stop->setColor(glm::vec3(.0f, .0f, .1f));
	});
	
	rgbButton_goToEnd->setInputUpdateCallback([this](){
		if(!Stacato::Editor::getCurrentProject()) return;
		auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
        if(!manoeuvre || !manoeuvre->canRapidToTarget()) return;
        if(!rgbButton_goToEnd->isPressed()) manoeuvre->rapidToTarget();
	});
	rgbButton_goToEnd->setOutputUpdateCallback([this](){
		        auto project = Stacato::Editor::getCurrentProject();
		if(project == nullptr) return;
		auto plot = project->getCurrentPlot();
		if(plot == nullptr) return;
		auto manoeuvre = plot->getSelectedManoeuvre();
        if(manoeuvre == nullptr) rgbButton_goToEnd->setColor(glm::vec3(0.f, 0.f, 0.f));
        else if(manoeuvre->isAtTarget()) rgbButton_goToEnd->setColor(glm::vec3(.0f, 1.f, .0f));
        else if(manoeuvre->canRapidToTarget()) rgbButton_goToEnd->setColor(glm::vec3(.1f, .1f, .1f));
        else rgbButton_goToEnd->setColor(glm::vec3(.0f, .0f, .1f));
	});
	
	
	
	//======= SCROLL BUTTONS ========
	
	pushbutton_arrowUp->setInputUpdateCallback([this](){
		if(!Stacato::Editor::getCurrentProject()) return;
		if(pushbutton_arrowUp->isPressed()) Stacato::Editor::getCurrentProject()->getCurrentPlot()->getManoeuvreList()->selectPreviousManoeuvre();
	});
	pushbutton_arrowDown->setInputUpdateCallback([this](){
		if(!Stacato::Editor::getCurrentProject()) return;
		if(pushbutton_arrowDown->isPressed()) Stacato::Editor::getCurrentProject()->getCurrentPlot()->getManoeuvreList()->selectNextManoeuvre();
	});
	
	
	//======= JOYSTICKS ========
	
	joystick3x_left->setInputUpdateCallback([this](){
		glm::vec3 pos = joystick3x_left->getPosition();
		leftJoystickControlChannel->setControlValue(pos.x, pos.y, pos.z);
	});
	joystick3x_right->setInputUpdateCallback([this](){
		glm::vec3 pos = joystick3x_right->getPosition();
		rightJoystickControlChannel->setControlValue(pos.x, pos.y, pos.z);
	});
	pushbutton_leftJoystickDeadman->setInputUpdateCallback([](){});
	pushbutton_rightJoystickDeadman->setInputUpdateCallback([](){});
	pushbutton_leftJoystickSelection->setInputUpdateCallback([this](){
		if(pushbutton_leftJoystickSelection->isPressed()) leftJoystickControlChannel->requestAxisSelectionPopupOpen(true);
	});
	pushbutton_rightJoystickSelection->setInputUpdateCallback([this](){
		if(pushbutton_rightJoystickSelection->isPressed()) rightJoystickControlChannel->requestAxisSelectionPopupOpen(true);
	});
}



void StacatoV2::onDisconnection(){
	leftJoystickControlChannel->setControlValue(0.0, 0.0, 0.0);
	rightJoystickControlChannel->setControlValue(0.0, 0.0, 0.0);
}

