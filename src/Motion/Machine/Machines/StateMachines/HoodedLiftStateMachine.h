#include "Motion/Machine/Machine.h"

#include "Motion/AnimatableParameter.h"

class HoodedLiftStateMachine : public Machine {
public:

	DEFINE_MACHINE_NODE(HoodedLiftStateMachine, "Hooded Lift State Machine", "HoodedLiftStateMachine");

	std::shared_ptr<NodePin> gpioDeviceLink = std::make_shared<NodePin>(NodeData::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "GPIO Device", NodePinFlags_NoDataField);

	std::shared_ptr<NodePin> hoodOpenSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Hood Open", NodePinFlags_NoDataField);
	std::shared_ptr<NodePin> hoodShutSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Hood Shut", NodePinFlags_NoDataField);
	std::shared_ptr<NodePin> liftRaisedSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Lift Raised", NodePinFlags_NoDataField);
	std::shared_ptr<NodePin> liftLoweredSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Lift Lowered", NodePinFlags_NoDataField);
	std::shared_ptr<NodePin> hoodMotorCircuitBreakerSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Hood Motor Fuse", NodePinFlags_NoDataField);
	std::shared_ptr<NodePin> liftMotorCircuitBreakerSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Lift Motor Fuse", NodePinFlags_NoDataField);
	std::shared_ptr<NodePin> emergencyStopClearSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Emergency Stop Clear", NodePinFlags_NoDataField);
	std::shared_ptr<NodePin> localControlEnabledSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Local Control Enabled", NodePinFlags_NoDataField);

	std::shared_ptr<NodePin> openHoodCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Open Hood", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> shutHoodCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Shut Hood", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> raiseLiftCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Raise Lift", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> lowerLiftCommandPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Lower Lift", NodePinFlags_DisableDataField);

	std::shared_ptr<AnimatableParameter> stateParameter;
	std::shared_ptr<AnimatableParameter> realParameter;
	static std::vector<StateParameterValue> stateParameterValues;

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

	bool b_enabled = false;

	virtual bool load(tinyxml2::XMLElement* xml) { return true; }
	virtual bool save(tinyxml2::XMLElement* xml) { return true; }
};