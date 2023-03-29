#include "Machine/Machine.h"

#include "Animation/Animatables/AnimatableState.h"

#include "Animation/AnimationValue.h"

#include "Gui/Environnement/Dashboard/Widget.h"

/*
class FlipStateMachine : public Machine {
public:

	DEFINE_MACHINE_NODE(FlipStateMachine, "Flip State Machine", "FlipStateMachine", "State Machines");

	//————— Input Pins —————
	
	std::shared_ptr<NodePin> gpioDeviceLink = std::make_shared<NodePin>(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_INPUT, "GPIO Device", NodePin::Flags::NoDataField);

	std::shared_ptr<bool> hoodOpenSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> hoodShutSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> liftRaisedSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> liftLoweredSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> hoodMotorCircuitBreakerSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> liftMotorCircuitBreakerSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> emergencyStopClearSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> localControlEnabledSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> hoodOpenSignalPin = std::make_shared<NodePin>(hoodOpenSignal,
																		   NodePin::Direction::NODE_INPUT, "Hood Open", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> hoodShutSignalPin = std::make_shared<NodePin>(hoodShutSignal,
																		   NodePin::Direction::NODE_INPUT, "Hood Shut", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> liftRaisedSignalPin = std::make_shared<NodePin>(liftRaisedSignal,
																			 NodePin::Direction::NODE_INPUT, "Lift Raised", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> liftLoweredSignalPin = std::make_shared<NodePin>(liftLoweredSignal,
																			  NodePin::Direction::NODE_INPUT, "Lift Lowered", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> hoodMotorCircuitBreakerSignalPin = std::make_shared<NodePin>(hoodMotorCircuitBreakerSignal,
																						  NodePin::Direction::NODE_INPUT, "Hood Motor Fuse", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> liftMotorCircuitBreakerSignalPin = std::make_shared<NodePin>(liftMotorCircuitBreakerSignal,
																						  NodePin::Direction::NODE_INPUT, "Lift Motor Fuse", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> emergencyStopClearSignalPin = std::make_shared<NodePin>(emergencyStopClearSignal,
																					 NodePin::Direction::NODE_INPUT, "Emergency Stop Clear", NodePin::Flags::NoDataField);
	std::shared_ptr<NodePin> localControlEnabledSignalPin = std::make_shared<NodePin>(localControlEnabledSignal,
																					  NodePin::Direction::NODE_INPUT, "Local Control Enabled", NodePin::Flags::NoDataField);
	
	//————— Output Pins —————
	
	std::shared_ptr<bool> openHoodSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> shutHoodSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> raiseLiftSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> lowerLiftSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> openHoodCommandPin = std::make_shared<NodePin>(openHoodSignal,
																			NodePin::Direction::NODE_OUTPUT, "Open Hood", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> shutHoodCommandPin = std::make_shared<NodePin>(shutHoodSignal,
																			NodePin::Direction::NODE_OUTPUT, "Shut Hood", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> raiseLiftCommandPin = std::make_shared<NodePin>(raiseLiftSignal,
																			 NodePin::Direction::NODE_OUTPUT, "Raise Lift", NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> lowerLiftCommandPin = std::make_shared<NodePin>(lowerLiftSignal,
																			 NodePin::Direction::NODE_OUTPUT, "Lower Lift", NodePin::Flags::DisableDataField);
	
	std::shared_ptr<int> stateIntegerValue = std::make_shared<int>(-1);
	std::shared_ptr<NodePin> stateIntegerPin = std::make_shared<NodePin>(stateIntegerValue,
																		 NodePin::Direction::NODE_OUTPUT, "State Number", NodePin::Flags::DisableDataField);
	
	//————— Machine State —————
	
	bool areAllPinsConnected();
	bool isGpioDeviceConnected();
	std::shared_ptr<GpioInterface> getGpioDevice();
	
	//————— State Machine —————
	
	static AnimatableStateStruct stateUnknown;
	static AnimatableStateStruct stateStopped;
	static AnimatableStateStruct stateClosed;
	static AnimatableStateStruct stateClosingOpening;
	static AnimatableStateStruct stateOpenLowered;
	static AnimatableStateStruct stateRaisingLowering;
	static AnimatableStateStruct stateRaised;
	
	static std::vector<AnimatableStateStruct*> allStates;
	static std::vector<AnimatableStateStruct*> selectableStates;

	enum class State{
		UNKNOWN,
		STOPPED,
		CLOSED,
		OPENING_CLOSING,
		OPEN_LOWERED,
		LOWERING_RAISING,
		RAISED
	};
	
	State actualState = State::CLOSED;
	State requestedState = State::STOPPED;
	
	int getStateInteger(State state){
		switch(state){
			case State::UNKNOWN:
			case State::STOPPED:			return -1;
			case State::CLOSED:				return 0;
			case State::OPENING_CLOSING:	return 1;
			case State::OPEN_LOWERED:		return 2;
			case State::LOWERING_RAISING:	return 3;
			case State::RAISED:				return 4;
		}
	}
	
	//————— Animation —————
	
	std::shared_ptr<AnimatableState> animatableState = AnimatableState::make("State", allStates, selectableStates, &stateStopped);
	void requestState(State newState);
	
	//————— Control Widget —————
	
	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); };
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); }
	void widgetGui();
	class ControlWidget : public Widget{
	public:
		ControlWidget(std::shared_ptr<FlipStateMachine> machine_) : Widget("Machines"), machine(machine_){}
		std::shared_ptr<FlipStateMachine> machine;
		virtual void gui() override;
		virtual std::string getName() override { return machine->getName(); }
	};
	std::shared_ptr<ControlWidget> controlWidget;
	
	
};


#define FlipStateMachineStrings {\
{FlipStateMachine::State::UNKNOWN, 			.displayString = "Unknown State"},\
{FlipStateMachine::State::STOPPED, 			.displayString = "Stopped"},\
{FlipStateMachine::State::CLOSED, 			.displayString = "Closed"},\
{FlipStateMachine::State::OPENING_CLOSING, 	.displayString = "Opening/Closing"},\
{FlipStateMachine::State::OPEN_LOWERED, 	.displayString = "Open & Lowered"},\
{FlipStateMachine::State::LOWERING_RAISING, .displayString = "Lowering/Raising"},\
{FlipStateMachine::State::RAISED, 			.displayString = "Raised"},\
}\

DEFINE_ENUMERATOR(FlipStateMachine::State, FlipStateMachineStrings)
*/
