#include "Motion/Machine/Machine.h"

#include "Motion/Curve/Curve.h"
#include "Motion/MotionTypes.h"

class PositionControlledAxis;

class PositionControlledSingleAxisMachine : public Machine{
	
	DEFINE_MACHINE_NODE(PositionControlledSingleAxisMachine, "Position Controlled Single Axis Machine", "PositionControlledSingleAxisMachine")

	std::shared_ptr<NodePin> positionControlledAxisLink = std::make_shared<NodePin>(NodeData::Type::POSITION_CONTROLLED_AXIS_LINK, DataDirection::NODE_INPUT, "Position Controlled Axis");
	
	std::shared_ptr<AnimatableParameter> positionParameter;

	bool b_enabled = false;

	bool isAxisConnected();
	std::shared_ptr<PositionControlledAxis> getAxis();

	//======== MACHINE SETTINGS =========

	double rapidVelocity_machineUnitsPerSecond = 0.0;
	double rapidAcceleration_machineUnitsPerSecond = 0.0;


	//======== SAVING & LOADING ========

	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);
};