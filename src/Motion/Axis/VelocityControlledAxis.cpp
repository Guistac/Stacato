#include <pch.h>

#include "VelocityControlledAxis.h"

#include "Motion/SubDevice.h"

#include <tinyxml2.h>

#include "Fieldbus/EtherCatFieldbus.h"

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
	
	//update profile time no matter what
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
	//exit the process loop if no actuator is connected
	if(!isActuatorDeviceConnected()) {
		*actualLoad = 0.0;
		*actualVelocity = 0.0;
		return;
	}
	auto actuator = getActuatorDevice();
	
	//handle disabling of the axis if the actuator gets disabled
	if(isEnabled() && !actuator->isEnabled()) disable();
	
	//get load
	*actualLoad = actuator->getLoad();
	
	if (isAxisPinConnected()) return;
	
	switch(controlMode){
		case ControlMode::VELOCITY_TARGET:
			motionProfile.matchVelocity(profileTimeDelta_seconds, manualVelocityTarget, manualAcceleration);
			break;
		case ControlMode::FAST_STOP:
			motionProfile.matchVelocity(profileTimeDelta_seconds, 0.0, accelerationLimit);
			break;
		default: break;
	}

	sendActuatorCommands();
}

//called by node connected to axis pin to send velocity to the axis
void VelocityControlledAxis::setVelocityCommand(double velocity){
	motionProfile.setVelocity(velocity);
	sendActuatorCommands();
}

void VelocityControlledAxis::sendActuatorCommands() {
	*actualVelocity = motionProfile.getVelocity();
	double velocity_actuatorUnits = axisUnitsToActuatorUnits(motionProfile.getVelocity());
	getActuatorDevice()->setVelocityCommand(velocity_actuatorUnits);
	actuatorPin->updateConnectedPins();
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
		using namespace std::chrono;
		system_clock::time_point enableTime = system_clock::now();
		
		std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();
		actuator->enable();
		while(system_clock::now() - enableTime < milliseconds(500)){
			if(actuator->isEnabled()){
				b_enabled = true;
				onEnable();
				Logger::info("Velocity Controlled Axis '{}' Enabled", getName());
				return;
			}
			std::this_thread::sleep_for(milliseconds(10));
		}
		
		b_enabled = false;
		actuator->disable();
		Logger::warn("Could not enable velocity controlled axis '{}'", getName());
	});
	axisEnabler.detach();
}

void VelocityControlledAxis::disable() {
	getActuatorDevice()->disable();
	b_enabled = false;
}

void VelocityControlledAxis::onEnable() {}
void VelocityControlledAxis::onDisable() {}

void VelocityControlledAxis::setVelocity(double velocity) {
	controlMode = ControlMode::VELOCITY_TARGET;
	manualVelocityTarget = velocity;
}

void VelocityControlledAxis::fastStop(){
	controlMode = ControlMode::FAST_STOP;
	manualVelocityTarget = 0.0;
}

void VelocityControlledAxis::setMovementType(MovementType type){
	movementType = type;
	switch(type){
		case MovementType::ROTARY:
			if(positionUnit->unitType != Units::Type::ANGULAR_DISTANCE){
				setPositionUnit(Units::AngularDistance::get().front());
			}
			break;
		case MovementType::LINEAR:
			if(positionUnit->unitType != Units::Type::LINEAR_DISTANCE){
				setPositionUnit(Units::LinearDistance::get().front());
			}
			break;
	}
}


void VelocityControlledAxis::setPositionUnit(Unit unit){
	positionUnit = unit;
}


void VelocityControlledAxis::sanitizeParameters(){
	actuatorUnitsPerAxisUnits = std::abs(actuatorUnitsPerAxisUnits);
	velocityLimit = std::min(std::abs(velocityLimit), actuatorUnitsToAxisUnits(getActuatorDevice()->getVelocityLimit()));
	accelerationLimit = std::min(std::abs(accelerationLimit), actuatorUnitsToAxisUnits(getActuatorDevice()->getAccelerationLimit()));
	manualAcceleration = std::min(std::abs(manualAcceleration), accelerationLimit);
}


bool VelocityControlledAxis::save(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* axisXML = xml->InsertNewChildElement("Axis");
	axisXML->SetAttribute("UnitType", Enumerator::getSaveString(movementType));
	axisXML->SetAttribute("Unit", positionUnit->saveString);
	
	XMLElement* unitConversionXML = xml->InsertNewChildElement("UnitConversion");
	unitConversionXML->SetAttribute("ActuatorUnitsPerAxisUnit", actuatorUnitsPerAxisUnits);

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("Limits");
	kinematicLimitsXML->SetAttribute("VelocityLimit", velocityLimit);
	kinematicLimitsXML->SetAttribute("AccelerationLimit", accelerationLimit);
	kinematicLimitsXML->SetAttribute("ManualAcceleration", manualAcceleration);

	return true;
}



bool VelocityControlledAxis::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* axisXML = xml->FirstChildElement("Axis");
	if (!axisXML) return Logger::warn("Could not load Machine Attributes");
	const char* axisUnitTypeString;
	if (axisXML->QueryStringAttribute("UnitType", &axisUnitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Axis Unit Type");
	if (!Enumerator::isValidSaveName<MovementType>(axisUnitTypeString)) return Logger::warn("Could not read Machine Unit Type");
	movementType = Enumerator::getEnumeratorFromSaveString<MovementType>(axisUnitTypeString);
	const char* axisUnitString;
	if (axisXML->QueryStringAttribute("Unit", &axisUnitString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit");
	switch(movementType){
		case MovementType::ROTARY:
			if(!Units::AngularDistance::isValidSaveString(axisUnitString)) return Logger::warn("Could not read Machine Unit");
			break;
		case MovementType::LINEAR:
			if(!Units::LinearDistance::isValidSaveString(axisUnitString)) return Logger::warn("Could not read Machine Unit");
			break;
	}
	positionUnit = Units::fromSaveString(axisUnitString);

	XMLElement* unitConversionXML = xml->FirstChildElement("UnitConversion");
	if (!unitConversionXML) return Logger::warn("Could not load Unit Conversion");
	if (unitConversionXML->QueryDoubleAttribute("ActuatorUnitsPerAxisUnit", &actuatorUnitsPerAxisUnits) != XML_SUCCESS) return Logger::warn("Could not load Actuator Units Per Axis Unit");

	XMLElement* kinematicLimitsXML = xml->FirstChildElement("Limits");
	if (!kinematicLimitsXML) return Logger::warn("Could not load Axis Kinematic Kimits");
	if (kinematicLimitsXML->QueryDoubleAttribute("VelocityLimit", &velocityLimit)) Logger::warn("Could not load velocity limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("AccelerationLimit", &accelerationLimit) != XML_SUCCESS) Logger::warn("Could not load acceleration limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("ManualAcceleration", &manualAcceleration) != XML_SUCCESS) Logger::warn("Could not load manual acceleration");
	
	return true;
}
