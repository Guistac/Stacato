#include "Machine.h"

class StateMachine : public Machine {
public:

	DEFINE_MACHINE_NODE("State Machine", StateMachine, Machine::Type::STATE_MACHINE);

	std::shared_ptr<NodePin> deviceLink = std::make_shared<NodePin>(NodeData::ACTUATOR_DEVICELINK, DataDirection::NODE_INPUT, "Actuators", NodePinFlags_AcceptMultipleInputs);
	std::shared_ptr<NodePin> state0ref = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 0 Feedback");
	std::shared_ptr<NodePin> state1ref = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 1 Feedback");
	std::shared_ptr<NodePin> state2ref = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 2 Feedback");

	std::shared_ptr<NodePin> state0Command = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> state1Command = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> state2Command = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", NodePinFlags_DisableDataField);

	virtual void assignIoData() {
		addIoData(deviceLink);
		addIoData(state0ref);
		addIoData(state1ref);
		addIoData(state2ref);

		addIoData(state0Command);
		addIoData(state1Command);
		addIoData(state2Command);
	}

	virtual void process() {}

	virtual bool load(tinyxml2::XMLElement* xml) { return true; }
	virtual bool save(tinyxml2::XMLElement* xml) { return true; }
};