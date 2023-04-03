#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Gui/Environnement/Dashboard/Widget.h"
#include "Project/Editor/Parameter.h"

class DeadMansSwitch : public Node {
public:
	
	DEFINE_NODE(DeadMansSwitch, "Dead Man's Switch", "DeadMansSwitch", Node::Type::PROCESSOR, "Safety")
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		Node::onCopyFrom(source);
	}
	
	enum class State{
		NOT_CONNECTED,
		NOT_PRESSED,
		PRESS_REQUESTED,
		PRESSED
	};
	
	bool isPressed(){ return state == State::PRESSED; }
	State getState(){ return state; }
	void requestPress(){ b_shouldRequestPress = true; }
	
private:
	
	State state = State::NOT_CONNECTED;
	long long pressRequestTime_nanoseconds;
	bool b_pressRequested = false;
	long long timeSincePressRequest_nanoseconds;
	double timeSincePressRequest_seconds = 0.0;
	
	bool b_shouldRequestPress = false;
    bool b_shouldKeepPressing = false;
	
	std::shared_ptr<NumberParameter<double>> requestTimeoutDelay = NumberParameter<double>::make(5.0,
																								 "Press Request Timeout Delay",
																								 "PressReuqestTimeoutDelay",
																								 "%.1f",
																								 Units::Time::Second,
																								 false);
	
	std::shared_ptr<NumberParameter<double>> requestBlinkFrequency = NumberParameter<double>::make(4.0,
																								   "Request Blink Frequency",
																								   "RequestBlinkFrequency",
																								   "%.1f",
																								   Units::Frequency::Hertz,
																								   false);
	
	std::shared_ptr<NumberParameter<double>> idleBlinkFrequency = NumberParameter<double>::make(0.5,
																								"Idle Blink Frequency",
																								"IdleBlinkFrequency",
																								"%.2f",
																								Units::Frequency::Hertz,
																								false);
	
	std::shared_ptr<NumberParameter<double>> idleBlinkLength = NumberParameter<double>::make(0.1,
																							 "Idle Blink Length",
																							 "IdleBlinkLength",
																							 "%.2f",
																							 Units::Time::Second,
																							 false);

	void handlePressRequest();
	void updateLedState();
	
	std::shared_ptr<bool> b_switchPressed = std::make_shared<bool>(false);
	std::shared_ptr<bool> b_switchLed = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_INPUT, "Gpio Device", "GpioDevicePin");
	std::shared_ptr<NodePin> switchPressedPin = std::make_shared<NodePin>(b_switchPressed, NodePin::Direction::NODE_INPUT, "Switch Pressed", "SwitchPressedPin");
	
	std::shared_ptr<NodePin> switchLedPin = std::make_shared<NodePin>(b_switchLed, NodePin::Direction::NODE_OUTPUT, "Switch LED Signal", "SwitchLEDSignalPin");
	std::shared_ptr<NodePin> deadMansSwitchLink = std::make_shared<NodePin>(NodePin::DataType::DEAD_MANS_SWITCH, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Dead Man's Switch");
	
	bool areAllInputsReady();
	
	virtual void inputProcess() override;
	virtual void outputProcess() override;
	
	virtual void nodeSpecificGui() override;
	
	virtual bool save(tinyxml2::XMLElement* xml) override;
	virtual bool load(tinyxml2::XMLElement* xml) override;
	
	virtual void onAddToNodeGraph() override { controlWidget->addToDictionnary(); };
	virtual void onRemoveFromNodeGraph() override { controlWidget->removeFromDictionnary(); };
		
	void widgetGui();
	
	class ControlWidget : public Widget{
	public:
		
		ControlWidget(std::shared_ptr<DeadMansSwitch> deadMansSwitch_) : Widget("Safety"), deadMansSwitch(deadMansSwitch_){}
		std::shared_ptr<DeadMansSwitch> deadMansSwitch;
		virtual void gui() override { deadMansSwitch->widgetGui(); }
		virtual std::string getName() override { return deadMansSwitch->getName(); }
	};
	std::shared_ptr<ControlWidget> controlWidget;
};
