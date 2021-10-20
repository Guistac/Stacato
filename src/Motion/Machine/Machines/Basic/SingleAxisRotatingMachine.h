#include "Motion/Machine/Machine.h"

class SingleAxisRotatingMachine : public Machine {

	DEFINE_MACHINE_NODE(SingleAxisRotatingMachine, "Single Axis Rotating Machine", "SingleAxisRotatingMachine")

	std::shared_ptr<NodePin> axisLink = std::make_shared<NodePin>(NodeData::Type::AXIS_LINK, DataDirection::NODE_INPUT, "Rotating Axis");

};