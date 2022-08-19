#include <pch.h>

#include "SafetySignal.h"
#include "Motion/SubDevice.h"

#include "Environnement/Environnement.h"

void SafetySignal::initialize(){
	addNodePin(gpioPin);
	addNodePin(safetyValidPin);
	addNodePin(resetSafetyFaultPin);
	std::shared_ptr<SafetySignal> thisSafetySignal = std::static_pointer_cast<SafetySignal>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisSafetySignal, getName());
}

void SafetySignal::inputProcess(){
	

}

void SafetySignal::outputProcess(){}





bool SafetySignal::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;

	return true;
}

bool SafetySignal::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	return true;
	
}

