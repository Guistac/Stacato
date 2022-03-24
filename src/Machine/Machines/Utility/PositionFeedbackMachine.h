#pragma once

#include "Machine/Machine.h"

class PositionFeedbackMachine : public Machine {
	
	DEFINE_MACHINE_NODE(PositionFeedbackMachine, "Position Feedback", "PositionFeedback", "Utility")
	
	std::shared_ptr<NodePin> positionFeedbackDevicePin = std::make_shared<NodePin>(NodePin::DataType::POSITIONFEEDBACK, NodePin::Direction::NODE_INPUT, "Feedback Device");
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "Position");
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "Velocity");
	
	std::shared_ptr<double> positionPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> velocityPinValue = std::make_shared<double>(0.0);
	
	PositionUnitType movementType = PositionUnitType::LINEAR;
	PositionUnit positionUnit = PositionUnit::METER;
	double machineUnitsPerFeedbackUnit = 1.0;
	double machineUnitOffset = 0.0;
	bool b_invertDirection = false;
	
	void setScalingPosition(double currentPosition_machineUnits);
	
	bool isFeedbackConnected();
	std::shared_ptr<PositionFeedbackDevice> getFeedbackDevice();
	
	double position = 0.0;
	double velocity = 0.0;
	
	double feedbackPositionToMachinePosition(double feedbackPosition);
	double feedbackVelocityToMachineVelocity(double feedbackVelocity);
	
	void setMovementType(PositionUnitType t);
	void setPositionUnit(PositionUnit u);
};
