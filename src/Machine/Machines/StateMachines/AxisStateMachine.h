#include "Machine/Machine.h"

#include "Animation/Animatables/AnimatableState.h"
#include "Animation/Animatables/AnimatableReal.h"

#include "Animation/AnimationValue.h"

#include "Gui/Environnement/Dashboard/Widget.h"


class AxisStateMachine : public Machine {
public:

	DEFINE_MACHINE_NODE(AxisStateMachine, "Axis State Machine", "AxisStateMachine", "State Machines");

	//————— Input Pins —————
	
	std::shared_ptr<NodePin> axisPin = std::make_shared<NodePin>(NodePin::DataType::AXIS_INTERFACE,
																 NodePin::Direction::NODE_INPUT_BIDIRECTIONAL,
																 "Velocity Controlled Axis",
																 "VelocityControlledAxis");
	
	//————— Output Pins —————
	
	std::shared_ptr<int> stateInteger = std::make_shared<int>(-1.0);
	std::shared_ptr<NodePin> statePin = std::make_shared<NodePin>(stateInteger, NodePin::Direction::NODE_OUTPUT, "State", NodePin::Flags::DisableDataField);
	
	//————— State Machine —————
		
	enum class State{
		UNKNOWN,
		STOPPED,
		MOVING_TO_POSITIVE_LIMIT,
		MOVING_TO_NEGATIVE_LIMIT,
		AT_POSITIVE_LIMIT,
		AT_NEGATIVE_LIMIT
	};
	
	State actualState = State::STOPPED;
	State requestedState = State::STOPPED;
	double velocityTarget = 0.0;
	void requestState(State newState);
	void requestVelocityNormalized(double velocity);
	
	//————— Animation —————
		
	std::shared_ptr<AnimatableState> animatableState = AnimatableState::make("State", allStates, selectableStates, &stateStopped);
	std::shared_ptr<AnimatableReal> animatableVelocity = AnimatableReal::make("Velocity", Units::None::None, "/s");
	
	double manualVelocitySliderValue = 0.0;
	double movementVelocity = 0.0;
	
	//————— Machine State —————
	
	bool areAllPinsConnected();
	bool isAxisConnected();
	std::shared_ptr<AxisInterface> getAxis();
	
	//————— Control Widget —————
	
	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); };
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); }
	void widgetGui();
	class ControlWidget : public Widget{
	public:
		ControlWidget(std::shared_ptr<AxisStateMachine> machine_) : Widget("Machines"), machine(machine_){}
		std::shared_ptr<AxisStateMachine> machine;
		virtual void gui() override;
		virtual std::string getName() override { return machine->getName(); }
	};
	std::shared_ptr<ControlWidget> controlWidget;
	
	NumberParam<double> minNegativeVelocity;
	NumberParam<double> maxNegativeVelocity;
	NumberParam<double> minPositiveVelocity;
	NumberParam<double> maxPositiveVelocity;
	
	static AnimatableStateStruct stateUnknown;
	static AnimatableStateStruct stateStopped;
	static AnimatableStateStruct stateMovingToNegativeLimit;
	static AnimatableStateStruct stateMovingToPositiveLimit;
	static AnimatableStateStruct stateNegativeLimit;
	static AnimatableStateStruct statePositiveLimit;
	static std::vector<AnimatableStateStruct*> allStates;
	static std::vector<AnimatableStateStruct*> selectableStates;
	
	int getStateInteger(State state){
		switch(state){
			case State::UNKNOWN:					return -3;
			case State::STOPPED:					return -1;
			case State::MOVING_TO_POSITIVE_LIMIT:	return 0;
			case State::MOVING_TO_NEGATIVE_LIMIT:	return 1;
			case State::AT_POSITIVE_LIMIT:			return 2;
			case State::AT_NEGATIVE_LIMIT:			return 3;
		}
	}
	
	AnimatableStateStruct* getStateStruct(State axisState){
		switch(axisState){
			case State::UNKNOWN: return &stateUnknown;
			case State::STOPPED: return &stateStopped;
			case State::MOVING_TO_POSITIVE_LIMIT: return &stateMovingToNegativeLimit;
			case State::MOVING_TO_NEGATIVE_LIMIT: return &stateMovingToNegativeLimit;
			case State::AT_POSITIVE_LIMIT: return &statePositiveLimit;
			case State::AT_NEGATIVE_LIMIT: return &stateNegativeLimit;
		}
	}

	State getStateEnumerator(AnimatableStateStruct* stateStruct){
		if(stateStruct == &stateStopped) return State::STOPPED;
		else if(stateStruct == &stateMovingToNegativeLimit) return State::MOVING_TO_POSITIVE_LIMIT;
		else if(stateStruct == &stateMovingToNegativeLimit) return State::MOVING_TO_NEGATIVE_LIMIT;
		else if(stateStruct == &statePositiveLimit) return State::AT_POSITIVE_LIMIT;
		else if(stateStruct == &stateNegativeLimit) return State::AT_NEGATIVE_LIMIT;
		return State::UNKNOWN;
	}
	
};
