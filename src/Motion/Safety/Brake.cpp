#include <pch.h>

#include "Brake.h"

#include "Motion/Interfaces.h"

//#include "Environnement/Environnement.h"


AnimatableStateStruct Brake::stateUnknown = 	AnimatableStateStruct{.integerEquivalent = -2,	.displayName = "Unknown", 		.saveName = "Unknown"};
AnimatableStateStruct Brake::stateInBetween =	AnimatableStateStruct{.integerEquivalent = -1, 	.displayName = "In Between",	.saveName = "InBetween"};
AnimatableStateStruct Brake::stateStopped = 	AnimatableStateStruct{.integerEquivalent = 0,	.displayName = "Stopped", 		.saveName = "Stopped"};
AnimatableStateStruct Brake::stateClosed = 		AnimatableStateStruct{.integerEquivalent = 1,	.displayName = "Closed", 		.saveName = "Closed"};
AnimatableStateStruct Brake::stateOpen = 		AnimatableStateStruct{.integerEquivalent = 2,	.displayName = "Open", 			.saveName = "Open"};
std::vector<AnimatableStateStruct*> Brake::allStates = {
	&Brake::stateUnknown,
	&Brake::stateInBetween,
	&Brake::stateStopped,
	&Brake::stateClosed,
	&Brake::stateOpen
};
std::vector<AnimatableStateStruct*> Brake::selectableStates = {
	&Brake::stateClosed,
	&Brake::stateOpen
};




void Brake::initialize(){
	
	addNodePin(gpio_Pin);
	addNodePin(brakeOpenStatus_Pin);
	addNodePin(brakeClosedStatus_Pin);
	addNodePin(brakeOpenControl_Pin);
	addNodePin(brakeClosedControl_Pin);
	
	addAnimatable(animatableState);
	
	controlWidget = std::make_shared<ControlWidget>(std::static_pointer_cast<Brake>(shared_from_this()));
}

std::string Brake::getStatusString(){
	return "no status string yet";
}

void Brake::inputProcess(){
	
	b_emergencyStopActive = false;
	b_halted = false;
	
	auto isBrakeOnline = [this]() -> bool{
		if(!gpio_Pin->isConnected()) return false;
		for(auto connectedGpioPin : gpio_Pin->getConnectedPins()){
			auto gpioDevice = connectedGpioPin->getSharedPointer<GpioInterface>();
			if(gpioDevice->getState() != DeviceState::ENABLED) return false;
		}
		if(!brakeOpenStatus_Pin->isConnected()) return false;
		if(!brakeClosedStatus_Pin->isConnected()) return false;
		return true;
	};
	
	if(!isBrakeOnline()){
		actualState = State::OFFLINE;
		//targetState = State::STOPPED;
		state = DeviceState::OFFLINE;
		animatableState->state = Animatable::State::OFFLINE;
		return;
	}else{
		state = DeviceState::ENABLED;
		animatableState->state = Animatable::State::READY;
	}
	
	brakeOpenStatus_Pin->copyConnectedPinValue();
	brakeClosedStatus_Pin->copyConnectedPinValue();
	
	bool b_isOpen = *brakeOpenStatus_Signal;
	bool b_isClosed = *brakeClosedStatus_Signal;
	
	if(b_isClosed && b_isOpen) actualState = State::UNKNOWN;
	else if(b_isOpen) actualState = State::OPEN;
	else if(b_isClosed) actualState = State::CLOSED;
	else actualState = State::IN_BETWEEN;
	
	
	auto actualStateValue = AnimationValue::makeState();
	switch(actualState){
		case State::UNKNOWN:	actualStateValue->value = &stateUnknown; break;
		case State::OPEN:		actualStateValue->value = &stateOpen; break;
		case State::CLOSED:		actualStateValue->value = &stateClosed; break;
		default: actualStateValue->value = &stateInBetween; break;
	}
	animatableState->updateActualValue(actualStateValue);
}

void Brake::outputProcess(){

	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
	
	b_halted = animatableState->isHalted();
	
	//update outputs signals
	if (!isEnabled()) {
		animatableState->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
		*brakeOpenControl_Signal = false;
		*brakeClosedControl_Signal = false;
		
	}else{
		
		animatableState->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
		auto targetValue = animatableState->getTargetValue()->toState()->value;
		
		if(targetValue == &stateClosed){
			*brakeOpenControl_Signal = false;
			*brakeClosedControl_Signal = true;
		}
		else if(targetValue == &stateOpen){
			*brakeOpenControl_Signal = true;
			*brakeClosedControl_Signal = false;
		}
		else{
			*brakeOpenControl_Signal = false;
			*brakeClosedControl_Signal = false;
		}
	}
	
}

void Brake::requestState(State newState){
	animatableState->stopMovement();
	auto targetValue = AnimationValue::makeState();
	switch(newState){
		case State::CLOSED:
			targetValue->value = &stateClosed;
			break;
		case State::OPEN:
			targetValue->value = &stateOpen;
			break;
		default:
			targetValue->value = &stateStopped;
			break;
	}
	if(targetValue) {
		animatableState->rapidToValue(targetValue);
		Logger::info("rapid {} to value {}", getName(), targetValue->value->displayName);
	}
}


bool Brake::isHardwareReady() {
	return true;
}

void Brake::enableHardware() {
}

void Brake::disableHardware() {
}


void Brake::onEnableHardware() {}

void Brake::onDisableHardware() {}

void Brake::simulateInputProcess() {
}

void Brake::simulateOutputProcess() {
}

bool Brake::isSimulationReady(){
	return true;
}


void Brake::onEnableSimulation() {}
void Brake::onDisableSimulation() {}


void Brake::fillAnimationDefaults(std::shared_ptr<Animation> animation){}


void Brake::getDevices(std::vector<std::shared_ptr<Device>>& output) {
}



bool Brake::saveMachine(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* controlWidgetXML = xml->InsertNewChildElement("ControlWidget");
	controlWidgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	
	return true;
}

bool Brake::loadMachine(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* controlWidgetXML;
	if(!loadXMLElement("ControlWidget", xml, controlWidgetXML)) return false;
	if(controlWidgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS){
		Logger::warn("could not load dead mans switch control widget unique id");
		return false;
	}
	
	return true;
}

bool Brake::isMoving(){}
