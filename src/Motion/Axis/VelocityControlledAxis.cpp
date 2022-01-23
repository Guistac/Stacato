#include <pch.h>

#include "VelocityControlledAxis.h"

#include "Motion/SubDevice.h"

#include <tinyxml2.h>

void VelocityControlledAxis::initialize() {
	//inputs
	addNodePin(actuatorPin);
	
	//outputs
	std::shared_ptr<VelocityControlledAxis> thisAxis = std::dynamic_pointer_cast<VelocityControlledAxis>(shared_from_this());
	velocityControlledAxisPin->assignData(thisAxis);
	addNodePin(velocityControlledAxisPin);
	addNodePin(velocityPin);
	addNodePin(loadPin);
}

void VelocityControlledAxis::process() {
	if (!isAxisPinConnected()) {
		//manual controls are available
		sendActuatorCommands();
	}
}

void VelocityControlledAxis::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isActuatorDeviceConnected()) output.push_back(getActuatorDevice()->parentDevice);
}

bool VelocityControlledAxis::isReady() {
	if (isActuatorDeviceConnected() && getActuatorDevice()->isReady()) return true;
	return false;
}

bool VelocityControlledAxis::isMoving() {
	return false;
	//return getActuatorDevice()->isMoving
}

void VelocityControlledAxis::enable() {
	if (!isReady()) return;
	std::thread axisEnabler([this]() {
		std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();
		actuator->enable();
		using namespace std::chrono;
		system_clock::time_point enableTime = system_clock::now();
		while (!actuator->isEnabled()) {
			if (system_clock::now() - enableTime > milliseconds(100)) {
				b_enabled = false;
				Logger::warn("Could not enable axis");
				return;
			}
		}
		b_enabled = true;
		Logger::info("Axis Enabled");
	});
	axisEnabler.detach();
}

void VelocityControlledAxis::disable() {
	getActuatorDevice()->disable();
	b_enabled = false;
}

void VelocityControlledAxis::onEnable() {}
void VelocityControlledAxis::onDisable() {}

void VelocityControlledAxis::sendActuatorCommands() {
	getActuatorDevice()->setVelocityCommand(motionProfile.getVelocity());
	//TODO: check if this actually works and updates the pins
	actuatorPin->updateConnectedPins();
}

void VelocityControlledAxis::setVelocity(double velocity_axisUnitsPerSecond) {
	controlMode = ControlMode::VELOCITY_TARGET;
	manualVelocityTarget_axisUnitsPerSecond = velocity_axisUnitsPerSecond;
}

void VelocityControlledAxis::fastStop(){
	controlMode = ControlMode::FAST_STOP;
	manualVelocityTarget_axisUnitsPerSecond = 0.0;
}

void VelocityControlledAxis::setPositionUnitType(PositionUnitType type){
	positionUnitType = type;
	switch(type){
		case PositionUnitType::ANGULAR:
			if(!isAngularPositionUnit(positionUnit)) {
				for(auto& type : Unit::getTypes<PositionUnit>()){
					if(isAngularPositionUnit(type.enumerator)){
						setPositionUnit(type.enumerator);
						break;
					}
				}
			}
			break;
		case PositionUnitType::LINEAR:
			if(!isLinearPositionUnit(positionUnit)){
				for(auto& type : Unit::getTypes<PositionUnit>()){
					if(isLinearPositionUnit(type.enumerator)){
						setPositionUnit(type.enumerator);
						break;
					}
				}
			}
			break;
	}
}


void VelocityControlledAxis::setPositionUnit(PositionUnit unit){
	positionUnit = unit;
}


void VelocityControlledAxis::sanitizeParameters(){
	actuatorUnitsPerAxisUnits = std::abs(actuatorUnitsPerAxisUnits);
	velocityLimit_axisUnitsPerSecond = std::min(std::abs(velocityLimit_axisUnitsPerSecond), actuatorUnitsToAxisUnits(getActuatorDevice()->getVelocityLimit()));
	accelerationLimit_axisUnitsPerSecondSquared = std::min(std::abs(accelerationLimit_axisUnitsPerSecondSquared), actuatorUnitsToAxisUnits(getActuatorDevice()->getAccelerationLimit()));
	manualControlAcceleration_axisUnitsPerSecond = std::min(std::abs(manualControlAcceleration_axisUnitsPerSecond), accelerationLimit_axisUnitsPerSecondSquared);
}


bool VelocityControlledAxis::save(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* axisXML = xml->InsertNewChildElement("Axis");
	axisXML->SetAttribute("UnitType", Enumerator::getSaveString(positionUnitType));
	axisXML->SetAttribute("Unit", Unit::getSaveString(positionUnit));
	
	XMLElement* unitConversionXML = xml->InsertNewChildElement("UnitConversion");
	unitConversionXML->SetAttribute("ActuatorUnitsPerAxisUnit", actuatorUnitsPerAxisUnits);

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit_axisUnitsPerSecond", velocityLimit_axisUnitsPerSecond);
	kinematicLimitsXML->SetAttribute("AccelerationLimit_axisUnitsPerSecondSquared", accelerationLimit_axisUnitsPerSecondSquared);

	return true;
}



bool VelocityControlledAxis::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* axisXML = xml->FirstChildElement("Axis");
	if (!axisXML) return Logger::warn("Could not load Machine Attributes");
	const char* axisUnitTypeString;
	if (axisXML->QueryStringAttribute("UnitType", &axisUnitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Axis Unit Type");
	if (!Enumerator::isValidSaveName<PositionUnitType>(axisUnitTypeString)) return Logger::warn("Could not read Machine Unit Type");
	positionUnitType = Enumerator::getEnumeratorFromSaveString<PositionUnitType>(axisUnitTypeString);
	const char* axisUnitString;
	if (axisXML->QueryStringAttribute("Unit", &axisUnitString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit");
	if (!Unit::isValidSaveName<PositionUnit>(axisUnitString)) return Logger::warn("Could not read Machine Unit");
	positionUnit = Unit::getEnumeratorFromSaveString<PositionUnit>(axisUnitString);

	XMLElement* unitConversionXML = xml->FirstChildElement("UnitConversion");
	if (!unitConversionXML) return Logger::warn("Could not load Unit Conversion");
	if (unitConversionXML->QueryDoubleAttribute("ActuatorUnitsPerAxisUnit", &actuatorUnitsPerAxisUnits) != XML_SUCCESS) return Logger::warn("Could not load Actuator Units Per Axis Unit");

	XMLElement* kinematicLimitsXML = xml->FirstChildElement("KinematicLimits");
	if (!kinematicLimitsXML) return Logger::warn("Could not load Axis Kinematic Kimits");
	if (kinematicLimitsXML->QueryDoubleAttribute("VelocityLimit_axisUnitsPerSecond", &velocityLimit_axisUnitsPerSecond)) Logger::warn("Could not load velocity limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("AccelerationLimit_axisUnitsPerSecondSquared", &accelerationLimit_axisUnitsPerSecondSquared) != XML_SUCCESS) Logger::warn("Could not load acceleration limit");
	
	return true;
}
