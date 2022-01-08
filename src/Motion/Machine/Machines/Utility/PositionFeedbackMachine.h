#pragma once

#include "Motion/Machine/Machine.h"

class PositionFeedbackMachine : public Machine {
	
	DEFINE_MACHINE_NODE(PositionFeedbackMachine, "Position Feedback", "PositionFeedback", "Utility")
	
	std::shared_ptr<NodePin> positionFeedbackDevicePin = std::make_shared<NodePin>(NodeData::Type::POSITIONFEEDBACK_DEVICELINK, DataDirection::NODE_INPUT, "Feedback Device");
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(NodeData::Type::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position");
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(NodeData::Type::REAL_VALUE, DataDirection::NODE_OUTPUT, "Velocity");
	
	PositionUnit::Type movementType = PositionUnit::Type::LINEAR;
	PositionUnit::Unit positionUnit = PositionUnit::Unit::METER;
	double unitsPerFeedbackUnit = 1.0;
	double unitOffset = 0.0;
	bool b_invertDirection = false;
	bool isFeedbackConnected();
	std::shared_ptr<PositionFeedbackDevice> getFeedbackDevice();
	
	double position = 0.0;
	double velocity = 0.0;
	
	double feedbackPositionToMachinePosition(double feedbackPosition);
	double feedbackVelocityToMachineVelocity(double feedbackVelocity);
};
