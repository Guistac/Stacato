#include <pch.h>

#include "Axis.h"

#include "NodeGraph/DeviceNode.h"

std::vector<AxisType> axisTypes = {
	{UnitType::ANGULAR, "Rotating Axis", "Rotating"},
	{UnitType::LINEAR, "Linear Axis", "Linear"}
};
std::vector<AxisType>& getAxisTypes() { return axisTypes; }
AxisType& getAxisType(UnitType t) {
	for (AxisType& axis : axisTypes) {
		if (axis.unitType == t) return axis;
	}
	return axisTypes.back();
}


std::vector<PositionUnit> linearPositionUnits = {
	{PositionUnit::Unit::METER, UnitType::LINEAR,		"Meter",		"Meters",		"Meter"},
	{PositionUnit::Unit::MILLIMETER, UnitType::LINEAR,	"Millimeter",	"Millimeters",	"Millimeter"}
};
std::vector<PositionUnit> angularPositionUnits = {
	{PositionUnit::Unit::DEGREE, UnitType::ANGULAR,		"Degree",		"Degrees",		"Degree"},
	{PositionUnit::Unit::RADIAN, UnitType::ANGULAR,		"Radian",		"Radians",		"Radian"},
	{PositionUnit::Unit::ROTATION, UnitType::ANGULAR,	"Rotation",		"Rotations",	"Rotation"}
};
std::vector<PositionUnit>& getLinearPositionUnits() { return linearPositionUnits; }
std::vector<PositionUnit>& getAngularPositionUnits() { return angularPositionUnits; }
PositionUnit& getPositionUnitType(PositionUnit::Unit u) {
	for (PositionUnit& unit : linearPositionUnits) {
		if (unit.unit == u) return unit;
	}
	for (PositionUnit& unit : angularPositionUnits) {
		if (unit.unit == u) return unit;
	}
	return linearPositionUnits.back();
}


std::vector<PositionFeedback> positionFeedbackTypes = {
	{PositionFeedback::Type::ABSOLUTE_FEEDBACK, "Absolute Feedback", "Absolute"},
	{PositionFeedback::Type::INCREMENTAL_FEEDBACK, "Incremental Feedback", "Incremental"},
	{PositionFeedback::Type::NO_FEEDBACK, "No Feedback", "None"}
};
std::vector<PositionFeedback>& getPositionFeedbackTypes() { return positionFeedbackTypes; }
PositionFeedback& getPositionFeedbackType(PositionFeedback::Type t) {
	for (PositionFeedback& feedback : positionFeedbackTypes) {
		if (feedback.type == t) return feedback;
	}
	return positionFeedbackTypes.back();
}


std::vector<PositionReference> positionReferenceTypes = {
	{PositionReference::Type::LOW_LIMIT, "Low Limit", "Low"},
	{PositionReference::Type::HIGH_LIMIT, "High Limit", "High"},
	{PositionReference::Type::LOW_AND_HIGH_LIMIT, "Low and High Limit", "LowHigh"},
	{PositionReference::Type::POSITION_REFERENCE, "Position Reference", "Reference"},
	{PositionReference::Type::NO_LIMIT, "No Limit", "None"}
};
std::vector<PositionReference>& getPositionReferenceTypes() { return positionReferenceTypes; }
PositionReference& getPositionReferenceType(PositionReference::Type t) {
	for (PositionReference& reference : positionReferenceTypes) {
		if (reference.type == t) return reference;
	}
	return positionReferenceTypes.back();
}

std::vector<CommandType> commandTypes = {
	{CommandType::Type::POSITION_COMMAND, "Position Command", "Position"},
	{CommandType::Type::VELOCITY_COMMAND, "Velocity Command", "Velocity"}
};
std::vector<CommandType>& getCommandTypes() { return commandTypes; }
CommandType& getCommandType(CommandType::Type t) {
	for (CommandType& command : commandTypes) {
		if (command.type == t) return command;
	}
	return commandTypes.back();
}


std::vector<HomingDirection> homingDirectionTypes = {
	{HomingDirection::Type::NEGATIVE, "Negative", "Negative"},
	{HomingDirection::Type::POSITIVE, "Positive", "Positive"},
	{HomingDirection::Type::DONT_CARE, "Don't Care", "DontCare"}
};
std::vector<HomingDirection>& getHomingDirectionTypes() { return homingDirectionTypes; }
HomingDirection& getHomingDirectionType(HomingDirection::Type t) {
	for (HomingDirection& direction : homingDirectionTypes) {
		if (direction.type == t) return direction;
	}
	return homingDirectionTypes.back();
}





void Axis::process() {
	double updateTime_seconds = Timing::getTime_seconds();
	double deltaT_seconds = updateTime_seconds - lastUpdateTime_seconds;

	if (b_enabled) {

		if (profileVelocity_degreesPerSecond != velocityControlTarget_degreesPerSecond) {
			double deltaV_degreesPerSecond;
			deltaV_degreesPerSecond = defaultMovementAcceleration_degreesPerSecondSquared * deltaT_seconds;
			if (profileVelocity_degreesPerSecond < velocityControlTarget_degreesPerSecond) {
				profileVelocity_degreesPerSecond += deltaV_degreesPerSecond;
				if (profileVelocity_degreesPerSecond > velocityControlTarget_degreesPerSecond) profileVelocity_degreesPerSecond = velocityControlTarget_degreesPerSecond;
			}
			else {
				profileVelocity_degreesPerSecond -= deltaV_degreesPerSecond;
				if (profileVelocity_degreesPerSecond < velocityControlTarget_degreesPerSecond) profileVelocity_degreesPerSecond = velocityControlTarget_degreesPerSecond;
			}
		}
		double deltaP_degrees = profileVelocity_degreesPerSecond * deltaT_seconds;

		profilePosition_degrees += deltaP_degrees;

		positionCommand->set(profilePosition_degrees);
	}
	else {
		positionCommand->set(positionFeedback->getLinks().front()->getInputData()->getReal());
	}

	lastUpdateTime_seconds = updateTime_seconds;
}

void Axis::enable() {
	if (deviceLink->isConnected()) {
		bool allConnectedDeviceAreEnabled = true;
		for (auto link : deviceLink->getLinks()) {
			std::shared_ptr<ioNode> inputNode = link->getInputData()->getNode();
			if (inputNode->getType() == NodeType::IODEVICE) {
				std::shared_ptr<DeviceNode> device = std::dynamic_pointer_cast<DeviceNode>(inputNode);
				if (!device->isEnabled()) {
					allConnectedDeviceAreEnabled = false;
					Logger::warn("Cannot Enable axis, DeviceNode {} is not enabled", device->getName());
				}
			}
			else {
				Logger::warn("Node {} Connected to axis Device Link is not a DeviceNode", inputNode->getName());
				return;
			}
		}
		if (allConnectedDeviceAreEnabled) {
			b_enabled = true;
			onEnable();
		}
	}
	else {
		Logger::warn("Cannot enable axis,No Node is Connected to axis Device Link");
	}
}

void Axis::onEnable() {
	profilePosition_degrees = positionFeedback->getLinks().front()->getInputData()->getReal();
	profileVelocity_degreesPerSecond = 0.0;
	profileAcceleration_degreesPerSecond = 0.0;
	Logger::warn("Axis {} enabled", getName());
}

void Axis::disable() {
	b_enabled = false;
	Logger::warn("Axis {} disabled", getName());
}

bool Axis::isEnabled() {
	return b_enabled;
}


bool Axis::areAllActuatorsEnabled() {
	if (deviceLink->isConnected()) {
		for (auto link : deviceLink->getLinks()) {
			std::shared_ptr<ioNode> inputNode = link->getInputData()->getNode();
			if (inputNode->getType() == NodeType::IODEVICE) {
				std::shared_ptr<DeviceNode> device = std::dynamic_pointer_cast<DeviceNode>(inputNode);
				if (!device->isEnabled()) return false;
			}
			else {
				Logger::warn("Node {} Connected to axis Device Link is not a DeviceNode", inputNode->getName());
				return false;
			}
		}
	} else Logger::warn("No Actuators Connected To Axis '{}'", getName());
	return false;
}

void Axis::enableAllActuators() {
	if (deviceLink->isConnected()) {
		for (auto link : deviceLink->getLinks()) {
			std::shared_ptr<ioNode> inputNode = link->getInputData()->getNode();
			if (inputNode->getType() == NodeType::IODEVICE) {
				std::shared_ptr<DeviceNode> device = std::dynamic_pointer_cast<DeviceNode>(inputNode);
				device->enable();
			}
			else {
				Logger::warn("Node {} Connected to axis Device Link is not a DeviceNode", inputNode->getName());
				return;
			}
		}
	}
	else Logger::warn("No Actuators Connected To Axis '{}'", getName());
}

void Axis::disableAllActuators() {
	if (deviceLink->isConnected()) {
		for (auto link : deviceLink->getLinks()) {
			std::shared_ptr<ioNode> inputNode = link->getInputData()->getNode();
			if (inputNode->getType() == NodeType::IODEVICE) {
				std::shared_ptr<DeviceNode> device = std::dynamic_pointer_cast<DeviceNode>(inputNode);
				device->disable();
			}
			else {
				Logger::warn("Node {} Connected to axis Device Link is not a DeviceNode", inputNode->getName());
				return;
			}
		}
	} else Logger::warn("No Actuators Connected To Axis '{}'", getName());
}