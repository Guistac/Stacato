#include <pch.h>

#include "PositionFeedback.h"

#include "Motion/SubDevice.h"

#include <tinyxml2.h>

/*

bool PositionFeedback::isFeedbackConnected(){
	return positionFeedbackDevicePin->isConnected();
}

std::shared_ptr<MotionFeedbackModule> PositionFeedback::getFeedbackDevice(){
	return positionFeedbackDevicePin->getConnectedPins().front()->getSharedPointer<MotionFeedbackModule>();
}

double PositionFeedback::feedbackPositionToMachinePosition(double feedbackPosition){
	if(b_invertDirection) return (-1.0 * feedbackPosition * machineUnitsPerFeedbackUnit) + machineUnitOffset;
	return (feedbackPosition * machineUnitsPerFeedbackUnit) + machineUnitOffset;
}

double PositionFeedback::feedbackVelocityToMachineVelocity(double feedbackVelocity){
	if(b_invertDirection) return (-1.0 * feedbackVelocity * machineUnitsPerFeedbackUnit);
	return (feedbackVelocity * machineUnitsPerFeedbackUnit);
}


void PositionFeedback::setMovementType(MovementType t){
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

void PositionFeedback::setPositionUnit(Unit u){
	positionUnit = u;
}


void PositionFeedback::initialize(){
	positionPin->assignData(positionPinValue);
	velocityPin->assignData(velocityPinValue);
	addNodePin(positionFeedbackDevicePin);
	addNodePin(positionPin);
	addNodePin(velocityPin);
}

void PositionFeedback::inputProcess(){
	if(isFeedbackConnected()){
		std::shared_ptr<PositionFeedbackDevice> feedback = getFeedbackDevice();
		*positionPinValue = feedbackPositionToMachinePosition(feedback->getPosition());
		*velocityPinValue = feedbackVelocityToMachineVelocity(feedback->getVelocity());
	}else{
		*positionPinValue = 0.0;
		*velocityPinValue = 0.0;
	}
}

void PositionFeedback::outputProcess(){
	Logger::critical("Output process not defined for position feedback machine");
	abort();
	//not applicable?
}


void PositionFeedback::setScalingPosition(double realPosition_machineUnits){
	machineUnitsPerFeedbackUnit = (realPosition_machineUnits - machineUnitOffset) / getFeedbackDevice()->getPosition();
}



bool PositionFeedback::save(tinyxml2::XMLElement* xml){
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

bool PositionFeedback::load(tinyxml2::XMLElement* xml){
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

*/
