#include "Motion/Machine/Machine.h"

class HoodedLiftStateMachine : public Machine {
public:

	DEFINE_MACHINE_NODE(HoodedLiftStateMachine, "Hooded Lift State Machine", "HoodedLiftStateMachine");

	std::shared_ptr<NodePin> gpioDeviceLink = std::make_shared<NodePin>(NodeData::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "GPIO Device");

	std::shared_ptr<NodePin> hoodShutSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Hood Shut");
	std::shared_ptr<NodePin> hoodOpenSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Hood Open");
	std::shared_ptr<NodePin> liftLoweredSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Lift Lowered");
	std::shared_ptr<NodePin> liftRaisedSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Lift Raised");
	std::shared_ptr<NodePin> emergencyStopSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Emergency Stop Active");
	std::shared_ptr<NodePin> remoteControlEnabledSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Remote Control Enabled");
	std::shared_ptr<NodePin> hoodMotorFuseSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Hood Motor Fuse");
	std::shared_ptr<NodePin> liftMotorFuseSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Lift Motor Fuse");

	std::shared_ptr<NodePin> shutLidCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Shut Lid Command", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> openLidCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Open Lid Command", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> lowerPlatformCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Lower Platform Commmand", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> raisePlatformCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Raise Platform Command", NodePinFlags_DisableDataField);

	bool hoodShut = false;
	bool hoodOpen = false;
	bool liftLowered = false;
	bool liftRaised = false;
	bool emergencyStopActive = false;
	bool remoteControlEnabled = false;
	bool hoodMotorFuseBurned = false;
	bool liftMotorFuseBurned = false;

	bool shutLid = false;
	bool openLid = false;
	bool lowerPlatform = false;
	bool raisePlatform = false;

	struct MachineState {
		enum class State{
			UNKNOWN,
			UNEXPECTED_STATE,
			LIFT_LOWERED_HOOD_SHUT,
			LIFT_LOWERED_HOOD_MOVING,
			LIFT_LOWERED_HOOD_OPEN,
			LIFT_MOVING_HOOD_OPEN,
			LIFT_RAISED_HOOD_OPEN
		};
		State state;
		const char displayName[64];
	};
	std::vector<MachineState>& getStates();
	MachineState* getState(MachineState::State s);

	bool isGpioDeviceConnected();
	std::shared_ptr<GpioDevice> getGpioDevice();
	void updateGpioInSignals();
	void updateGpioOutSignals();

	MachineState::State actualState = MachineState::State::UNKNOWN;
	MachineState::State requestedState = MachineState::State::UNKNOWN;

	bool b_enabled = false;

	virtual bool load(tinyxml2::XMLElement* xml) { return true; }
	virtual bool save(tinyxml2::XMLElement* xml) { return true; }
};