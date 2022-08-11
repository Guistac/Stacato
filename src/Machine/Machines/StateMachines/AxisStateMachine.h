#include "Machine/Machine.h"

#include "Animation/Animatables/AnimatableState.h"

#include "Animation/AnimationValue.h"

#include "Gui/Environnement/Dashboard/Widget.h"

class AxisStateMachine : public Machine {
public:

	DEFINE_MACHINE_NODE(AxisStateMachine, "Axis State Machine", "AxisStateMachine", "State Machines");

	//————— Input Pins —————
	
	std::shared_ptr<NodePin> axisPin = std::make_shared<NodePin>(NodePin::DataType::VELOCITY_CONTROLLED_AXIS,
																 NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Velocity Controlled Axis", "VelocityControlledAxis");
	
	//————— Output Pins —————
	
	std::shared_ptr<int> stateInteger = std::make_shared<int>(-1.0);
	std::shared_ptr<NodePin> statePin = std::make_shared<NodePin>(stateInteger, NodePin::Direction::NODE_OUTPUT, "State", NodePin::Flags::DisableDataField);
	
	//————— Machine State —————
	
	bool areAllPinsConnected();
	bool isAxisConnected();
	std::shared_ptr<VelocityControlledAxis> getAxis();
	
	//————— State Machine —————
	
	static AnimatableStateStruct stateUnknown;
	static AnimatableStateStruct stateStopped;
	static AnimatableStateStruct stateMovingToNegativeLimit;
	static AnimatableStateStruct stateMovingToPositiveLimit;
	static AnimatableStateStruct stateNegativeLimit;
	static AnimatableStateStruct statePositiveLimit;
	
	static std::vector<AnimatableStateStruct*> allStates;
	static std::vector<AnimatableStateStruct*> selectableStates;

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
	
	int getStateInteger(State state){
		switch(state){
			case State::UNKNOWN:					return -2;
			case State::STOPPED:					return -1;
			case State::MOVING_TO_POSITIVE_LIMIT:	return 0;
			case State::MOVING_TO_NEGATIVE_LIMIT:	return 1;
			case State::AT_POSITIVE_LIMIT:			return 2;
			case State::AT_NEGATIVE_LIMIT:			return 3;
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
		ControlWidget(std::shared_ptr<AxisStateMachine> machine_, std::string name) : Widget(name, "Machines"), machine(machine_){}
		std::shared_ptr<AxisStateMachine> machine;
		virtual void gui() override;
		virtual bool hasFixedContentSize() override{ return true; }
		virtual glm::vec2 getFixedContentSize() override;
	};
	std::shared_ptr<ControlWidget> controlWidget;
	
	
};
