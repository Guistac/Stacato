#include <pch.h>

#include "PositionFeedbackMachine.h"

#include "Motion/SubDevice.h"

#include <tinyxml2.h>

bool PositionFeedbackMachine::isFeedbackConnected(){
	return positionFeedbackDevicePin->isConnected();
}

std::shared_ptr<PositionFeedbackDevice> PositionFeedbackMachine::getFeedbackDevice(){
	return positionFeedbackDevicePin->getConnectedPins().front()->getSharedPointer<PositionFeedbackDevice>();
}

double PositionFeedbackMachine::feedbackPositionToMachinePosition(double feedbackPosition){
	if(b_invertDirection) return (-1.0 * feedbackPosition * machineUnitsPerFeedbackUnit) + machineUnitOffset;
	return (feedbackPosition * machineUnitsPerFeedbackUnit) + machineUnitOffset;
}

double PositionFeedbackMachine::feedbackVelocityToMachineVelocity(double feedbackVelocity){
	if(b_invertDirection) return (-1.0 * feedbackVelocity * machineUnitsPerFeedbackUnit);
	return (feedbackVelocity * machineUnitsPerFeedbackUnit);
}


void PositionFeedbackMachine::setMovementType(MovementType t){
	movementType = t;
	switch(movementType){
		case MovementType::LINEAR:
			if(positionUnit->unitType != Units::Type::LINEAR_DISTANCE){
				setPositionUnit(Units::LinearDistance::get().front());
			}
			break;
		case MovementType::ROTARY:
			if(positionUnit->unitType != Units::Type::ANGULAR_DISTANCE){
				setPositionUnit(Units::AngularDistance::get().front());
			}
			break;
	}
}

void PositionFeedbackMachine::setPositionUnit(Unit u){
	positionUnit = u;
}


void PositionFeedbackMachine::initialize(){
	positionPin->assignData(positionPinValue);
	velocityPin->assignData(velocityPinValue);
	addNodePin(positionFeedbackDevicePin);
	addNodePin(positionPin);
	addNodePin(velocityPin);
}

void PositionFeedbackMachine::inputProcess(){
	if(isFeedbackConnected() && isEnabled()){
		std::shared_ptr<PositionFeedbackDevice> feedback = getFeedbackDevice();
		*positionPinValue = feedbackPositionToMachinePosition(feedback->getPosition());
		*velocityPinValue = feedbackVelocityToMachineVelocity(feedback->getVelocity());
	}else{
		*positionPinValue = 0.0;
		*velocityPinValue = 0.0;
	}
}

void PositionFeedbackMachine::outputProcess(){
	Logger::critical("Output process not defined for position feedback machine");
	abort();
	//not applicable?
}

bool PositionFeedbackMachine::isMoving(){
	if(isFeedbackConnected()){
		auto feedback = getFeedbackDevice();
		if(feedback->isReady()) return feedback->isMoving();
	}
	return false;
}

bool PositionFeedbackMachine::isHardwareReady(){
	return isFeedbackConnected() && getFeedbackDevice()->isReady();
}

bool PositionFeedbackMachine::isSimulationReady(){
	return isFeedbackConnected();
}

void PositionFeedbackMachine::enableHardware(){
	b_enabled = true;
}

void PositionFeedbackMachine::disableHardware(){
	b_enabled = false;
}

void PositionFeedbackMachine::onEnableHardware(){
	//nothing to do here...
}

void PositionFeedbackMachine::onDisableHardware(){
	//nothing to do here...
}

void PositionFeedbackMachine::onEnableSimulation(){
	//nothing to do here...
}

void PositionFeedbackMachine::onDisableSimulation(){
	//nothing to do here...
}

void PositionFeedbackMachine::simulateInputProcess(){
	*positionPinValue = 0.0;
	*velocityPinValue = 0.0;
}

void PositionFeedbackMachine::simulateOutputProcess(){}



void PositionFeedbackMachine::setScalingPosition(double realPosition_machineUnits){
	machineUnitsPerFeedbackUnit = (realPosition_machineUnits - machineUnitOffset) / getFeedbackDevice()->getPosition();
}



bool PositionFeedbackMachine::saveMachine(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* unitsXML = xml->InsertNewChildElement("Units");
	unitsXML->SetAttribute("Type", Enumerator::getSaveString(movementType));
	unitsXML->SetAttribute("Unit", positionUnit->saveString);
	XMLElement* conversionXML = xml->InsertNewChildElement("Conversion");
	conversionXML->SetAttribute("UnitsPerFeedbackUnit", machineUnitsPerFeedbackUnit);
	conversionXML->SetAttribute("InvertDirectionOfMotion", b_invertDirection);
	conversionXML->SetAttribute("UnitOffset", machineUnitOffset);
	
	return true;
}

bool PositionFeedbackMachine::loadMachine(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* unitsXML = xml->FirstChildElement("Units");
	if(unitsXML == nullptr) return Logger::warn("Could not find units attribute");
	const char* unitTypeString;
	if(unitsXML->QueryStringAttribute("Type", &unitTypeString) != XML_SUCCESS) return Logger::warn("could not find unit type attribute");
	if(!Enumerator::isValidSaveName<MovementType>(unitTypeString)) return Logger::warn("Could not identify Position Unit Type");
	movementType = Enumerator::getEnumeratorFromSaveString<MovementType>(unitTypeString);
	const char* positionUnitString;
	if(unitsXML->QueryStringAttribute("Unit", &positionUnitString) != XML_SUCCESS) return Logger::warn("Could not find unit attribute");
	switch(movementType){
		case MovementType::ROTARY:
			if(!Units::AngularDistance::isValidSaveString(positionUnitString)) return false;
			break;
		case MovementType::LINEAR:
			if(!Units::LinearDistance::isValidSaveString(positionUnitString)) return false;
			break;
	}
	positionUnit = Units::fromSaveString(positionUnitString);
	
	XMLElement* conversionXML = xml->FirstChildElement("Conversion");
	if(conversionXML == nullptr) return Logger::warn("Could not find conversio attribute");
	if(conversionXML->QueryDoubleAttribute("UnitsPerFeedbackUnit", &machineUnitsPerFeedbackUnit) != XML_SUCCESS) return Logger::warn("Could not find units per feedback unit");
	if(conversionXML->QueryBoolAttribute("InvertDirectionOfMotion", &b_invertDirection) != XML_SUCCESS) return Logger::warn("Could not find invert direction");
	if(conversionXML->QueryDoubleAttribute("UnitOffset", &machineUnitOffset) != XML_SUCCESS) return Logger::warn("Could not find unit offset");
	
	return true;
}

void PositionFeedbackMachine::getDevices(std::vector<std::shared_ptr<Device>>& output){
	if(isFeedbackConnected()) output.push_back(getFeedbackDevice()->parentDevice);
}

void PositionFeedbackMachine::rapidAnimatableToValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value){}
void PositionFeedbackMachine::cancelAnimatableRapid(std::shared_ptr<Animatable> animatable){}
float PositionFeedbackMachine::getAnimatableRapidProgress(std::shared_ptr<Animatable> animatable){ return 0.0; }
bool PositionFeedbackMachine::isAnimatableReadyToStartPlaybackFromValue(std::shared_ptr<Animatable> Animatable, std::shared_ptr<AnimationValue> value){ return false; }
void PositionFeedbackMachine::onAnimationPlaybackStart(std::shared_ptr<Animatable> animatable){}
void PositionFeedbackMachine::onAnimationPlaybackInterrupt(std::shared_ptr<Animatable> animatable){}
void PositionFeedbackMachine::onAnimationPlaybackEnd(std::shared_ptr<Animatable> animatable){}
std::shared_ptr<AnimationValue> PositionFeedbackMachine::getActualAnimatableValue(std::shared_ptr<Animatable> animatable){}
void PositionFeedbackMachine::fillAnimationDefaults(std::shared_ptr<Animation> aniamtion){}
bool PositionFeedbackMachine::validateAnimation(const std::shared_ptr<Animation> animation){ return false; }
bool PositionFeedbackMachine::generateTargetAnimation(std::shared_ptr<TargetAnimation> targetAnimation){ return false; }
