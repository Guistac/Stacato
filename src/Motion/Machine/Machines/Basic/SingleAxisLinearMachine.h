#include "Motion/Machine/Machine.h"

class SingleAxisLinearMachine : public Machine{
	
	DEFINE_MACHINE_NODE(SingleAxisLinearMachine, "Single Axis Linear Machine", "SingleAxisLinearMachine")

	std::shared_ptr<NodePin> axisLink = std::make_shared<NodePin>(NodeData::Type::AXIS_LINK, DataDirection::NODE_INPUT, "Linear Axis");

};