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


void PositionFeedbackMachine::setMovementType(Unit::DistanceType t){
	movementType = t;
	switch(movementType){
		case Unit::DistanceType::LINEAR:
			if(!Unit::isLinearDistance(positionUnit)){
				for(auto& type : Unit::getUnits<Unit::Distance>()){
					if(Unit::isLinearDistance(type.enumerator)) setPositionUnit(type.enumerator);
					break;
				}
			}
			break;
		case Unit::DistanceType::ANGULAR:
			if(!Unit::isAngularDistance(positionUnit)){
				for(auto& type : Unit::getUnits<Unit::Distance>()){
					if(Unit::isAngularDistance(type.enumerator)) setPositionUnit(type.enumerator);
					break;
				}
			}
			break;
	}
}

void PositionFeedbackMachine::setPositionUnit(Unit::Distance u){
	switch(movementType){
		case Unit::DistanceType::ANGULAR:
			if(!Unit::isAngularDistance(u)) return;
			break;
		case Unit::DistanceType::LINEAR:
			if(!Unit::isLinearDistance(u)) return;
			break;
	}
	positionUnit = u;
}


void PositionFeedbackMachine::initialize(){
	positionPin->assignData(positionPinValue);
	velocityPin->assignData(velocityPinValue);
	addNodePin(positionFeedbackDevicePin);
	addNodePin(positionPin);
	addNodePin(velocityPin);
}

void PositionFeedbackMachine::process(){
	if(isFeedbackConnected() && isEnabled()){
		std::shared_ptr<PositionFeedbackDevice> feedback = getFeedbackDevice();
		*positionPinValue = feedbackPositionToMachinePosition(feedback->getPosition());
		*velocityPinValue = feedbackVelocityToMachineVelocity(feedback->getVelocity());
	}else{
		*positionPinValue = 0.0;
		*velocityPinValue = 0.0;
	}
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

void PositionFeedbackMachine::simulateProcess(){
	*positionPinValue = 0.0;
	*velocityPinValue = 0.0;
}



void PositionFeedbackMachine::setScalingPosition(double realPosition_machineUnits){
	machineUnitsPerFeedbackUnit = (realPosition_machineUnits - machineUnitOffset) / getFeedbackDevice()->getPosition();
}



bool PositionFeedbackMachine::saveMachine(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* unitsXML = xml->InsertNewChildElement("Units");
	unitsXML->SetAttribute("Type", Enumerator::getSaveString(movementType));
	unitsXML->SetAttribute("Unit", Unit::getSaveString(positionUnit));
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
	if(!Enumerator::isValidSaveName<Unit::DistanceType>(unitTypeString)) return Logger::warn("Could not identify Position Unit Type");
	movementType = Enumerator::getEnumeratorFromSaveString<Unit::DistanceType>(unitTypeString);
	const char* positionUnitString;
	if(unitsXML->QueryStringAttribute("Unit", &positionUnitString) != XML_SUCCESS) return Logger::warn("Could not find unit attribute");
	if(!Unit::isValidSaveName<Unit::Distance>(positionUnitString)) return Logger::warn("Could not identify position unit");
	positionUnit = Unit::getEnumeratorFromSaveString<Unit::Distance>(positionUnitString);
	
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

void PositionFeedbackMachine::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value){}
void PositionFeedbackMachine::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter){}
float PositionFeedbackMachine::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter){
	return 0.0;
}
bool PositionFeedbackMachine::isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value){
	return false;
}
void PositionFeedbackMachine::onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter){}
void PositionFeedbackMachine::onParameterPlaybackInterrupt(std::shared_ptr<AnimatableParameter> parameter){}
void PositionFeedbackMachine::onParameterPlaybackEnd(std::shared_ptr<AnimatableParameter> parameter){}
void PositionFeedbackMachine::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value){}
bool PositionFeedbackMachine::validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack){
	return false;
}
void PositionFeedbackMachine::getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves){}
bool PositionFeedbackMachine::getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit){
	return false;
}
