#include <pch.h>

#include "DeadMansSwitch.h"
#include "Motion/Interfaces.h"

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"

void DeadMansSwitch::onConstruction(){
	Node::onConstruction();
	
	setName("Dead Man's Switch");
	
	gpioDevicePin = NodePin::createInstance(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_INPUT, "Gpio Device", "GpioDevicePin");
	switchPressedPin = NodePin::createInstance(b_switchPressed, NodePin::Direction::NODE_INPUT, "Switch Pressed", "SwitchPressedPin");
	
	switchLedPin = NodePin::createInstance(b_switchLed, NodePin::Direction::NODE_OUTPUT, "Switch LED Signal", "SwitchLEDSignalPin");
	deadMansSwitchLink = NodePin::createInstance(NodePin::DataType::DEAD_MANS_SWITCH, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Dead Man's Switch", "DeadManSwitch");
	
	addNodePin(gpioDevicePin);
	addNodePin(switchPressedPin);
	addNodePin(switchLedPin);
	std::shared_ptr<DeadMansSwitch> thisDeadMansSwitch = std::static_pointer_cast<DeadMansSwitch>(shared_from_this());
	deadMansSwitchLink->assignData(thisDeadMansSwitch);
	addNodePin(deadMansSwitchLink);
	
	requestTimeoutDelay = Legato::NumberParameter<double>::createInstance(5.0, "Press Request Timeout Delay", "PressRequestTimeoutDelay");
	requestTimeoutDelay->setUnit(Units::Time::Second);
	requestTimeoutDelay->setFormat("%.1f");
	
	requestBlinkFrequency = Legato::NumberParameter<double>::createInstance(4.0, "Request Blink Frequency", "RequestBlinkFrequency");
	requestBlinkFrequency->setUnit(Units::Frequency::Hertz);
	requestBlinkFrequency->setFormat("%.1f");
	
	idleBlinkFrequency = Legato::NumberParameter<double>::createInstance(0.5, "Idle Blink Frequency", "IdleBlinkFrequency");
	idleBlinkFrequency->setUnit(Units::Frequency::Hertz);
	idleBlinkFrequency->setFormat("%.2f");
	
	idleBlinkLength = Legato::NumberParameter<double>::createInstance(0.1, "Idle Blink Length", "IdleBlinkLength");
	idleBlinkLength->setUnit(Units::Time::Second);
	idleBlinkLength->setFormat("%.2f");
	
	controlWidget = std::make_shared<ControlWidget>(thisDeadMansSwitch);
}

bool DeadMansSwitch::areAllInputsReady(){
	if(!gpioDevicePin->isConnected()) return false;
	auto gpioDevice = gpioDevicePin->getConnectedPin()->getSharedPointer<GpioInterface>();
	if(!gpioDevice->isReady()) return false;
	if(!switchPressedPin->isConnected()) return false;
	return true;
}

void DeadMansSwitch::inputProcess(){
	
	if(!areAllInputsReady()){
		state = State::NOT_CONNECTED;
		b_pressRequested = false;
		return;
	}
	
	//update inputs
	switchPressedPin->copyConnectedPinValue();
	
    
	assert(false || "Environnement Timing should be accessed in a different way");
	/*
	if(b_pressRequested && !*b_switchPressed){
		//press request timeout detection
		long long time_nanoseconds = Environnement::getTime_nanoseconds();
		timeSincePressRequest_nanoseconds = time_nanoseconds - pressRequestTime_nanoseconds;
		timeSincePressRequest_seconds = timeSincePressRequest_nanoseconds / 1000000000.0;
		if(timeSincePressRequest_nanoseconds > requestTimeoutDelay->value * 1000000000.0){
			b_pressRequested = false;
		}
	}else if(b_pressRequested && *b_switchPressed){
		//when the switch is pressed, cancel the press request
		b_pressRequested = false;
	}
	 */
	
	//update switch state
	if(*b_switchPressed) state = State::PRESSED;
	else if(b_pressRequested) state = State::PRESS_REQUESTED;
	else state = State::NOT_PRESSED;
	
	handlePressRequest();
	updateLedState();
}

void DeadMansSwitch::outputProcess(){
    bool b_anyMachineMoving = false;
    for(auto connectedPin : deadMansSwitchLink->getConnectedPins()){
        auto node = connectedPin->getNode();
        //if(node->getType() != Node::Type::MACHINE) continue;
		/*
        auto machine = std::static_pointer_cast<Machine>(node);
        if(machine->isMoving()){
            b_anyMachineMoving = true;
            break;
        }
		*/
    }
    b_shouldKeepPressing = b_anyMachineMoving;
}

void DeadMansSwitch::handlePressRequest(){
	assert(false || "Environnement Timing should be accessed in a different way");
	/*
	//handle press request
	if(b_shouldRequestPress){
		b_shouldRequestPress = false;
		pressRequestTime_nanoseconds = Environnement::getTime_nanoseconds();
		b_pressRequested = true;
	}
	 */
}

void DeadMansSwitch::updateLedState(){
	assert(false || "Environnement Timing should be accessed in a different way");
	/*
	if(*b_switchPressed){
        if(b_shouldKeepPressing) *b_switchLed = Timing::getBlink(1.0 / requestBlinkFrequency->value);
        else *b_switchLed = true;
	}
    else if(b_pressRequested){
		double blinkPeriod = 1.0 / requestBlinkFrequency->value;
		*b_switchLed = fmod(timeSincePressRequest_seconds, blinkPeriod) < blinkPeriod * .5;
	}
    else{
		long long time_nanoseconds = Environnement::getTime_nanoseconds();
		long long blinkPeriod = 1000000000 / idleBlinkFrequency->value;
		long long blinkLength = 1000000000 * idleBlinkLength->value;
		*b_switchLed = time_nanoseconds % blinkPeriod < blinkLength;
	}
	 */
}





bool DeadMansSwitch::onSerialization(){
	bool success = true;
	success &= Node::onSerialization();
	
	success &= requestTimeoutDelay->serializeIntoParent(this);
	success &= requestBlinkFrequency->serializeIntoParent(this);
	success &= idleBlinkFrequency->serializeIntoParent(this);
	success &= idleBlinkLength->serializeIntoParent(this);
	
	success &= serializeAttribute("ControlWidgetID", controlWidget->uniqueID);
	
	return success;
}

bool DeadMansSwitch::onDeserialization(){
	bool success = true;
	success &= Node::onDeserialization();
	
	success &= requestTimeoutDelay->deserializeFromParent(this);
	success &= requestBlinkFrequency->deserializeFromParent(this);
	success &= idleBlinkFrequency->deserializeFromParent(this);
	success &= idleBlinkLength->deserializeFromParent(this);
	
	success &= deserializeAttribute("ControlWidgetID", controlWidget->uniqueID);
	
	return success;
}

