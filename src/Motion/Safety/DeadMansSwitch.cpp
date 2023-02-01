#include <pch.h>

#include "DeadMansSwitch.h"
#include "Motion/SubDevice.h"

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"

void DeadMansSwitch::initialize(){
	addNodePin(gpioDevicePin);
	addNodePin(switchPressedPin);
	addNodePin(switchLedPin);
	std::shared_ptr<DeadMansSwitch> thisDeadMansSwitch = std::static_pointer_cast<DeadMansSwitch>(shared_from_this());
	deadMansSwitchLink->assignData(thisDeadMansSwitch);
	addNodePin(deadMansSwitchLink);
	
	controlWidget = std::make_shared<ControlWidget>(thisDeadMansSwitch);
}

bool DeadMansSwitch::areAllInputsReady(){
	if(!gpioDevicePin->isConnected()) return false;
	auto gpioDevice = gpioDevicePin->getConnectedPin()->getSharedPointer<GpioModule>();
	if(!gpioDevice->isReady()) return false;
	if(!switchPressedPin->isConnected()) return false;
	return true;
}

void DeadMansSwitch::inputProcess(){
	
	if(!areAllInputsReady()){
		state = State::NOT_CONNECTED;
		b_pressRequested = false;
		return;
	}
	
	//update inputs
	switchPressedPin->copyConnectedPinValue();
	
    
    
	if(b_pressRequested && !*b_switchPressed){
		//press request timeout detection
		long long time_nanoseconds = Environnement::getTime_nanoseconds();
		timeSincePressRequest_nanoseconds = time_nanoseconds - pressRequestTime_nanoseconds;
		timeSincePressRequest_seconds = timeSincePressRequest_nanoseconds / 1000000000.0;
		if(timeSincePressRequest_nanoseconds > requestTimeoutDelay->value * 1000000000.0){
			b_pressRequested = false;
		}
	}else if(b_pressRequested && *b_switchPressed){
		//when the switch is pressed, cancel the press request
		b_pressRequested = false;
	}
    
	
	//update switch state
	if(*b_switchPressed) state = State::PRESSED;
	else if(b_pressRequested) state = State::PRESS_REQUESTED;
	else state = State::NOT_PRESSED;
	
	handlePressRequest();
	updateLedState();
}

void DeadMansSwitch::outputProcess(){
    bool b_anyMachineMoving = false;
    for(auto connectedPin : deadMansSwitchLink->getConnectedPins()){
        auto node = connectedPin->getNode();
        if(node->getType() != Node::Type::MACHINE) continue;
        auto machine = std::static_pointer_cast<Machine>(node);
        if(machine->isMoving()){
            b_anyMachineMoving = true;
            break;
        }
    }
    b_shouldKeepPressing = b_anyMachineMoving;
}

void DeadMansSwitch::handlePressRequest(){
	//handle press request
	if(b_shouldRequestPress){
		b_shouldRequestPress = false;
		pressRequestTime_nanoseconds = Environnement::getTime_nanoseconds();
		b_pressRequested = true;
	}
}

void DeadMansSwitch::updateLedState(){
	if(*b_switchPressed){
        if(b_shouldKeepPressing) *b_switchLed = Timing::getBlink(1.0 / requestBlinkFrequency->value);
        else *b_switchLed = true;
	}
    else if(b_pressRequested){
		double blinkPeriod = 1.0 / requestBlinkFrequency->value;
		*b_switchLed = fmod(timeSincePressRequest_seconds, blinkPeriod) < blinkPeriod * .5;
	}
    else{
		long long time_nanoseconds = Environnement::getTime_nanoseconds();
		long long blinkPeriod = 1000000000 / idleBlinkFrequency->value;
		long long blinkLength = 1000000000 * idleBlinkLength->value;
		*b_switchLed = time_nanoseconds % blinkPeriod < blinkLength;
	}
}





bool DeadMansSwitch::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* settings = xml->InsertNewChildElement("Settings");
	
	requestTimeoutDelay->save(settings);
	requestBlinkFrequency->save(settings);
	idleBlinkFrequency->save(settings);
	idleBlinkLength->save(settings);
	
	XMLElement* controlWidgetXML = xml->InsertNewChildElement("ControlWidget");
	controlWidgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);

	return true;
}

bool DeadMansSwitch::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* controlWidgetXML = xml->FirstChildElement("ControlWidget");
	if(!controlWidgetXML) {
		Logger::warn("could not find control widget attribute of dead mans switch");
		return false;
	}
	
	if(controlWidgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS){
		Logger::warn("could not load dead mans switch control widget unique id");
		return false;
	}
	
	XMLElement* settingsXml = xml->FirstChildElement("Settings");
	if(!settingsXml) {
		Logger::warn("Could not find attribute Settings of dead mans switch");
		return false;
	}
	
	if(!requestTimeoutDelay->load(settingsXml)){
		Logger::warn("Could not load attribute requestTimeoutDelay of dead mans switch");
		return false;
	}
	if(!requestBlinkFrequency->load(settingsXml)){
		Logger::warn("Could not load attribute requestBlinkFrequency of dead mans switch");
		return false;
	}
	if(!idleBlinkFrequency->load(settingsXml)){
		Logger::warn("Could not load attribute idleBlinkFrequency of dead mans switch");
		return false;
	}
	if(!idleBlinkLength->load(settingsXml)){
		Logger::warn("Could not load attribute idleBlinkLength of dead mans switch");
		return false;
	}
	
	return true;
	
}

