#pragma once

#include "Machine/Machine.h"
#include "Gui/Environnement/Dashboard/Widget.h"



class Brake : public Machine{
public:
	
	DEFINE_MACHINE_NODE(Brake, "Brake", "Brake", "Safety");
	
	
	std::shared_ptr<bool> brakeOpenStatus_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> brakeClosedStatus_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> brakeOpenControl_Signal = std::make_shared<bool>(false);
	std::shared_ptr<bool> brakeClosedControl_Signal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpio_Pin = std::make_shared<NodePin>(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_INPUT,
																 "Gpio Device", "GpioDevice",
																 NodePin::Flags::AcceptMultipleInputs);
	
	std::shared_ptr<NodePin> brakeOpenStatus_Pin = std::make_shared<NodePin>(brakeOpenStatus_Signal, NodePin::Direction::NODE_INPUT,
																			 "Brake Open Status", "BrakeOpenStatus",
																			 NodePin::Flags::DisableDataField);
	std::shared_ptr<NodePin> brakeClosedStatus_Pin = std::make_shared<NodePin>(brakeClosedStatus_Signal, NodePin::Direction::NODE_INPUT,
																			   "Brake Closed Status", "BrakeClosedStatus",
																			   NodePin::Flags::DisableDataField);
	
	std::shared_ptr<NodePin> brakeOpenControl_Pin = std::make_shared<NodePin>(brakeOpenControl_Signal, NodePin::Direction::NODE_OUTPUT,
																			  "Brake Open Control", "BrakeOpenControl");
	std::shared_ptr<NodePin> brakeClosedControl_Pin = std::make_shared<NodePin>(brakeClosedControl_Signal, NodePin::Direction::NODE_OUTPUT,
																				"Brake Closed Control", "BrakeClosedControl");
	
	static AnimatableStateStruct stateUnknown;
	static AnimatableStateStruct stateInBetween;
	static AnimatableStateStruct stateStopped;
	static AnimatableStateStruct stateClosed;
	static AnimatableStateStruct stateOpen;
	
	static std::vector<AnimatableStateStruct*> allStates;
	static std::vector<AnimatableStateStruct*> selectableStates;

	enum class State{
		OFFLINE,
		UNKNOWN,
		IN_BETWEEN,
		STOPPED,
		CLOSED,
		OPEN
	};
	
	State actualState = State::OFFLINE;
		
	//————— Animation —————
	
	std::shared_ptr<AnimatableState> animatableState = AnimatableState::make("State", allStates, selectableStates, &stateClosed);
	void requestState(State newState);

	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); };
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); };
		
	void widgetGui(bool b_withTitle);
	
private:
	
	class ControlWidget : public Widget{
	public:
		ControlWidget(std::shared_ptr<Brake> brake_) : Widget("Brake"), brake(brake_){}
		std::shared_ptr<Brake> brake;
		virtual void gui() override;
		virtual std::string getName() override {
			return brake->getName();
		}
	};
	std::shared_ptr<ControlWidget> controlWidget;
};
