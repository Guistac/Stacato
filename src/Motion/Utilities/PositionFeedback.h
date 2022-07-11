#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Motion/MotionTypes.h"

class PositionFeedback : public Node {
public:
	DEFINE_NODE(PositionFeedback, "Position Feedback", "PositionFeedback", Node::Type::PROCESSOR, "Motion/Utilities");
	
	std::shared_ptr<NodePin> positionFeedbackDevicePin = std::make_shared<NodePin>(NodePin::DataType::POSITION_FEEDBACK, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Feedback Device");
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "Position");
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "Velocity");
	
	std::shared_ptr<double> positionPinValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> velocityPinValue = std::make_shared<double>(0.0);
	
	MovementType movementType = MovementType::LINEAR;
	Unit positionUnit = Units::LinearDistance::Meter;
	double machineUnitsPerFeedbackUnit = 1.0;
	double machineUnitOffset = 0.0;
	bool b_invertDirection = false;
	
	virtual void inputProcess() override;
	virtual void outputProcess() override;
	
	void setScalingPosition(double currentPosition_machineUnits);
	
	bool isFeedbackConnected();
	std::shared_ptr<PositionFeedbackDevice> getFeedbackDevice();
	
	double position = 0.0;
	double velocity = 0.0;
	
	double feedbackPositionToMachinePosition(double feedbackPosition);
	double feedbackVelocityToMachineVelocity(double feedbackVelocity);
	
	void setMovementType(MovementType t);
	void setPositionUnit(Unit u);
	
	bool save(tinyxml2::XMLElement* xml) override;
	bool load(tinyxml2::XMLElement* xml) override;
	
	virtual void nodeSpecificGui() override;
	void controlsGui();
	void settingsGui();
};
