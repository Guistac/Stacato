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

static glm::vec3 lerpColor(glm::vec3 start, glm::vec3 end, float progress){
	return glm::vec3(start.x + (end.x - start.x) * progress,
					 start.y + (end.y - start.y) * progress,
					 start.z + (end.z - start.z) * progress);
};

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
	
	
	
	//——— Console Connection Status
	pwmLed1->setOutputUpdateCallback([&](){
		pwmLed1->setBrightness(Timing::getSinusWave(1.0, 0.0, 1.0));
	});
	//——— Environnement Status
	pwmLed2->setOutputUpdateCallback([&](){
		if(Environnement::isRunning()) pwmLed2->setBrightness(1.0);
		else if(Environnement::isStarting()) pwmLed2->setBrightness(Timing::getTriangleWave(.3, 0, 1));
		else pwmLed2->setBrightness(0.0);
	});
	//——— Machine Power Status
	pwmLed3->setOutputUpdateCallback([&](){
		if(Environnement::areAllMachinesEnabled()) pwmLed3->setBrightness(1.0);
		else if(Environnement::areNoMachinesEnabled()) pwmLed3->setBrightness(0.0);
		else pwmLed3->setBrightness(Timing::getBlink(.5) ? 1.0 : 0.0);
	});
	
	
	
	
	//——— Axis 1 Selection
	rgbButton1->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 1 : {}", rgbButton1->isPressed());
		if(rgbButton1->isPressed()) Environnement::toggleManualControlOfMachineIndex(0);
	});
	rgbButton1->setOutputUpdateCallback([&](){
		if(Environnement::isManualControlEnabledForMachineIndex(0)) rgbButton1->setColor(glm::vec3(1.0, 1.0, 1.0));
		else rgbButton1->setColor(glm::vec3(0.0, 0.0, 0.0));
	});
	
	
	
	
	//——— Axis 2 Selection
	rgbButton2->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 2 : {}", rgbButton2->isPressed());
		if(rgbButton2->isPressed()) Environnement::toggleManualControlOfMachineIndex(1);
	});
	rgbButton2->setOutputUpdateCallback([&](){
		if(Environnement::isManualControlEnabledForMachineIndex(1)) rgbButton2->setColor(glm::vec3(1.0, 1.0, 1.0));
		else rgbButton2->setColor(glm::vec3(0.0, 0.0, 0.0));
	});
	
	
	
	
	//——— Axis 3 Selection
	rgbButton3->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 3 : {}", rgbButton3->isPressed());
		if(rgbButton3->isPressed()) Environnement::toggleManualControlOfMachineIndex(2);
	});
	rgbButton3->setOutputUpdateCallback([&](){
		if(Environnement::isManualControlEnabledForMachineIndex(2)) rgbButton3->setColor(glm::vec3(1.0, 1.0, 1.0));
		else rgbButton3->setColor(glm::vec3(0.0, 0.0, 0.0));
	});
	
	
	
	
	//——— Playback speed adjustement toggle
	rgbButton4->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 4 : {}", rgbButton4->isPressed());
		if(rgbButton4->isPressed()){
			b_playbackSpeedAdjustButtonPressed = true;
			Environnement::getManualControlChannel()->setControlValue(0.0);
		}
		else{
			b_playbackSpeedAdjustButtonPressed = false;
		}
	});
	rgbButton4->setOutputUpdateCallback([&](){
		float br = Timing::getSinusWave(0.2, 0.0, 1.0);
		if(b_playbackSpeedAdjustButtonPressed) rgbButton4->setColor(glm::vec3(br,br,br));
		else rgbButton4->setColor(glm::vec3(0,0,0));
	});
	
	
	
	
	//——— Rapid To Start
	rgbButton5->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 5 : {}", rgbButton5->isPressed());
		if(!Stacato::Editor::hasCurrentProject()) return;
		auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
		if(!manoeuvre || !manoeuvre->canRapidToStart()) return;
		if(!rgbButton5->isPressed()) manoeuvre->rapidToStart();
	});
	rgbButton5->setOutputUpdateCallback([&](){
		if(!Stacato::Editor::hasCurrentProject()) return;
		if(auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre()){
			if(manoeuvre->isAtStart()) rgbButton5->setColor(glm::vec3(.0f, 1.f, .0f));
			else if(manoeuvre->canRapidToStart()) rgbButton5->setColor(glm::vec3(.1f, .1f, .1f));
			else rgbButton5->setColor(glm::vec3(0));
		}
		else rgbButton5->setColor(glm::vec3(0));
	});
	
	
	
	//——— Rapid To End
	rgbButton8->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 8 : {}", rgbButton8->isPressed());
		if(!Stacato::Editor::hasCurrentProject()) return;
		auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
		if(!manoeuvre || !manoeuvre->canRapidToTarget()) return;
		if(!rgbButton8->isPressed()) manoeuvre->rapidToTarget();
	});
	rgbButton8->setOutputUpdateCallback([&](){
		if(!Stacato::Editor::hasCurrentProject()) return;
		if(auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre()){
			if(manoeuvre->isAtTarget()) rgbButton8->setColor(glm::vec3(.0f, 1.f, .0f));
			else if(manoeuvre->canRapidToTarget()) rgbButton8->setColor(glm::vec3(.1f, .1f, .1f));
			else rgbButton8->setColor(glm::vec3(0));
		}
		else rgbButton8->setColor(glm::vec3(0));
	});
	
	
	
	//——— Stop
	rgbButton6->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 6 : {}", rgbButton6->isPressed());
		if(!Stacato::Editor::hasCurrentProject()) return;
		auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
		if(rgbButton6->isPressed()){
			if(manoeuvre && manoeuvre->canStop()) {
				manoeuvre->stop();
			}
			else if(PlaybackManager::isAnyAnimationActive()) PlaybackManager::stopAllAnimations();
		}
	});
	rgbButton6->setOutputUpdateCallback([&](){
		if(!Stacato::Editor::hasCurrentProject()) return;
		auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
		if(manoeuvre != nullptr && manoeuvre->canStop()){
			glm::vec3 black = glm::vec3(0.f, 0.f, 0.f);
			glm::vec3 red = glm::vec3(1.f, 0.f, 0.f);
			float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
			rgbButton6->setColor(lerpColor(black, red, lerp));
		}
		else if(PlaybackManager::isAnyAnimationActive()) rgbButton6->setColor(glm::vec3(1.f, .0f, .0f));
		else rgbButton6->setColor(glm::vec3(0));
	});
	
	
	
	//——— Play Pause
	rgbButton7->setInputUpdateCallback([&](){
		Logger::trace("RGB Button 7 : {}", rgbButton7->isPressed());
		if(!Stacato::Editor::hasCurrentProject()) return;
		auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
		if(!manoeuvre) return;
		if(!rgbButton7->isPressed()) {
			if(manoeuvre->canPausePlayback()) manoeuvre->pausePlayback();
			else if(manoeuvre->canStartPlayback()) manoeuvre->startPlayback();
			else if(manoeuvre->canRapidToPlaybackPosition()) manoeuvre->rapidToPlaybackPosition();
		}
	});
	rgbButton7->setOutputUpdateCallback([&](){
		if(!Stacato::Editor::hasCurrentProject()) return;
		 auto manoeuvre = Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre();
		 if(manoeuvre == nullptr) rgbButton7->setColor(glm::vec3(0.f, 0.f, 0.1f));
		 else if(manoeuvre->isPlaying()){
			 glm::vec3 black = glm::vec3(0.f);
			 glm::vec3 green = glm::vec3(0.f, 1.f, 0.f);
			 float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
			 rgbButton7->setColor(lerpColor(black, green, lerp));
		 }else if(manoeuvre->isInRapid()){
			 glm::vec3 white = glm::vec3(1.f);
			 glm::vec3 yellow = glm::vec3(1.f, .7f, 0.f);
			 float lerp = (1.f + std::sin(Timing::getProgramTime_seconds() * 20.0)) * .5f;
			 rgbButton7->setColor(lerpColor(white, yellow, lerp));
		 }
		 else if(manoeuvre->canStartPlayback()) rgbButton7->setColor(glm::vec3(0.1f, 0.1f, 0.1f));
		 else if(manoeuvre->canRapidToPlaybackPosition()) rgbButton7->setColor(glm::vec3(1.f, .7f, 0.f));
		 else rgbButton7->setColor(glm::vec3(0));
	});
	

	
	
	
	
	//——— Start Stop Environnement
	pushButton1->setInputUpdateCallback([&](){
		Logger::trace("Button 1 : {}", pushButton1->isPressed());
		if(!pushButton1->isPressed()){
			if(Environnement::isRunning()) Environnement::stop();
			else Environnement::start();
		}
	});
	//——— Enable All Machines
	pushButton2->setInputUpdateCallback([&](){
		Logger::trace("Button 2 : {}", pushButton2->isPressed());
		if(!pushButton2->isPressed()) Environnement::enableAllMachines();
	});
	//——— Disable All Machines
	pushButton3->setInputUpdateCallback([&](){
		Logger::trace("Button 3 : {}", pushButton3->isPressed());
		if(!pushButton3->isPressed()) Environnement::disableAllMachines();
	});
	//——— Select Previous Manoeuvre
	pushButton4->setInputUpdateCallback([&](){
		Logger::trace("Button 4 : {}", pushButton4->isPressed());
		if(!Stacato::Editor::hasCurrentProject()) return;
		if(pushButton4->isPressed()) Stacato::Editor::getCurrentProject()->getCurrentPlot()->getManoeuvreList()->selectPreviousManoeuvre();
	});
	//——— Select Next Manoeuvre
	pushButton5->setInputUpdateCallback([&](){
		Logger::trace("Button 5 : {}", pushButton5->isPressed());
		if(!Stacato::Editor::hasCurrentProject()) return;
		if(pushButton5->isPressed()) Stacato::Editor::getCurrentProject()->getCurrentPlot()->getManoeuvreList()->selectNextManoeuvre();
	});
	
	
	joystick->setInputUpdateCallback([&](){
		if(b_playbackSpeedAdjustButtonPressed){
			double min = PlaybackManager::getMinPlaybackSpeedMultiplier();
			double max = PlaybackManager::getMaxPlaybackSpeedMutliplier();
			double output = ((joystick->getPosition().x * 0.5) + 0.5) * (max - min) + min;
			PlaybackManager::setPlaybackSpeedMultiplier(output);
		}else{
			Environnement::getManualControlChannel()->setControlValue(joystick->getPosition().y);
		}
	});
	

    

	


	
}


void StacatoCompact::onDisconnection(){
	Environnement::getManualControlChannel()->setControlValue(0.0);
}
