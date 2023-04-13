#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Gui/Environnement/Dashboard/Widget.h"

#include "Legato/Editor/Parameters.h"

class DeadMansSwitch : public Node {
public:
	
	DEFINE_NODE(DeadMansSwitch, Node::Type::PROCESSOR, "Safety")
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override{
		Node::onCopyFrom(source);
	}
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
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
	
	Legato::NumberParam<double> requestTimeoutDelay;
	Legato::NumberParam<double> requestBlinkFrequency;
	Legato::NumberParam<double> idleBlinkFrequency;
	Legato::NumberParam<double> idleBlinkLength;
	
	void handlePressRequest();
	void updateLedState();
	
	std::shared_ptr<bool> b_switchPressed = std::make_shared<bool>(false);
	std::shared_ptr<bool> b_switchLed = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpioDevicePin;
	std::shared_ptr<NodePin> switchPressedPin;
	
	std::shared_ptr<NodePin> switchLedPin;
	std::shared_ptr<NodePin> deadMansSwitchLink;
	
	bool areAllInputsReady();
	
	virtual void inputProcess() override;
	virtual void outputProcess() override;
	
	virtual void nodeSpecificGui() override;
	
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
