#include "Machine/Machine.h"
#include "Machine/AnimatableParameter.h"

class HoodedLiftStateMachine : public Machine {
public:

	DEFINE_MACHINE_NODE(HoodedLiftStateMachine, "Hooded Lift State Machine", "HoodedLiftStateMachine", "State Machines");

	std::shared_ptr<NodePin> gpioDeviceLink = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "GPIO Device", NodePin::Flags::NoDataField);

	std::shared_ptr<NodePin> hoodOpenSignalPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Hood Open", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> hoodShutSignalPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Hood Shut", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> liftRaisedSignalPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Lift Raised", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> liftLoweredSignalPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Lift Lowered", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> hoodMotorCircuitBreakerSignalPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Hood Motor Fuse", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> liftMotorCircuitBreakerSignalPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Lift Motor Fuse", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> emergencyStopClearSignalPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Emergency Stop Clear", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> localControlEnabledSignalPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Local Control Enabled", NodePin::Flags::NoDataField);

	std::shared_ptr<bool> hoodOpenSignalPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> hoodShutSignalPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> liftRaisedSignalPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> liftLoweredSignalPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> hoodMotorCircuitBreakerSignalPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> liftMotorCircuitBreakerSignalPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> emergencyStopClearSignalPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> localControlEnabledSignalPinValue = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> openHoodCommandPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Open Hood", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> shutHoodCommandPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Shut Hood", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> raiseLiftCommandPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Raise Lift", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> lowerLiftCommandPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Lower Lift", NodePin::Flags::DisableDataField);

	std::shared_ptr<bool> openHoodCommandPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> shutHoodCommandPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> raiseLiftCommandPinValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> lowerLiftCommandPinValue = std::make_shared<bool>(false);
	
	std::shared_ptr<AnimatableStateParameter> stateParameter = std::make_shared<AnimatableStateParameter>("State", &stateParameterValues);
	static std::vector<AnimatableParameterState> stateParameterValues;

	bool hoodShut = false;
	bool hoodOpen = false;
	bool liftLowered = false;
	bool liftRaised = false;
	bool emergencyStopClear = false;
	bool localControlEnabled = false;
	bool hoodMotorCircuitBreakerTripped = false;
	bool liftMotorCircuitBreakerTripped = false;

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
		float floatEquivalent;
		const char displayName[64];
	};
	std::vector<MachineState>& getStates();
	MachineState* getState(MachineState::State s);

	bool isGpioDeviceConnected();
	std::shared_ptr<GpioDevice> getGpioDevice();
	void updateGpioInSignals();
	void updateGpioOutSignals();
	bool areGpioSignalsReady();

	MachineState::State actualState = MachineState::State::UNKNOWN;
	MachineState::State requestedState = MachineState::State::UNKNOWN;

	//used to track the progress of a requested parameter movement
	MachineState::State parameterMovementStartState;
	MachineState::State parameterMovementTargetState;
};
