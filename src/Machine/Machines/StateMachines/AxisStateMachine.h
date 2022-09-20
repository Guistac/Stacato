#include "Machine/Machine.h"

#include "Animation/Animatables/AnimatableState.h"
#include "Animation/Animatables/AnimatableReal.h"

#include "Animation/AnimationValue.h"

#include "Gui/Environnement/Dashboard/Widget.h"

class AxisStateMachine : public Machine {
public:

	DEFINE_MACHINE_NODE(AxisStateMachine, "Axis State Machine", "AxisStateMachine", "State Machines");

	//————— Input Pins —————
	
	std::shared_ptr<NodePin> axisPin = std::make_shared<NodePin>(NodePin::DataType::VELOCITY_CONTROLLED_AXIS,
																 NodePin::Direction::NODE_INPUT_BIDIRECTIONAL,
																 "Velocity Controlled Axis",
																 "VelocityControlledAxis");
	
	//————— Output Pins —————
	
	std::shared_ptr<int> stateInteger = std::make_shared<int>(-1.0);
	std::shared_ptr<NodePin> statePin = std::make_shared<NodePin>(stateInteger, NodePin::Direction::NODE_OUTPUT, "State", NodePin::Flags::DisableDataField);
	
	//————— Animation —————
	
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
	void requestState(State newState);
	
	std::shared_ptr<AnimatableReal> animatableVelocity = AnimatableReal::make("Velocity", Units::None::None, "/s");
	
	double manualVelocitySliderValue = 0.0;
	double movementVelocity = 0.0;
	
	//————— Machine State —————
	
	bool areAllPinsConnected();
	bool isAxisConnected();
	std::shared_ptr<VelocityControlledAxis> getAxis();
	
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
	
	
};

#define axisStateMachineStrings \
{.enumerator = AxisStateMachine::State::UNKNOWN,                  .displayString = "", .saveString = ""},\
{.enumerator = AxisStateMachine::State::STOPPED,                  .displayString = "", .saveString = ""},\
{.enumerator = AxisStateMachine::State::MOVING_TO_POSITIVE_LIMIT, .displayString = "", .saveString = ""},\
{.enumerator = AxisStateMachine::State::MOVING_TO_NEGATIVE_LIMIT, .displayString = "", .saveString = ""},\
{.enumerator = AxisStateMachine::State::AT_POSITIVE_LIMIT,        .displayString = "", .saveString = ""},\
{.enumerator = AxisStateMachine::State::AT_NEGATIVE_LIMIT,        .displayString = "", .saveString = ""}\

DEFINE_ENUMERATOR(AxisStateMachine::State, axisStateMachineStrings)
