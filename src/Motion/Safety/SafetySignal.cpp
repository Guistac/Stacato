#include <pch.h>

#include "SafetySignal.h"
#include "Motion/Interfaces.h"

#include "Environnement/Environnement.h"

void SafetySignal::onConstruction(){
	Node::onConstruction();
	
	setName("Safety Signal");
	
	gpioPin = NodePin::createInstance(NodePin::DataType::GPIO_INTERFACE, NodePin::Direction::NODE_INPUT,
									  "Gpio Device", "GpioDevice", NodePin::Flags::AcceptMultipleInputs);
	safetyLineValidPin = NodePin::createInstance(safetyLineValidSignal, NodePin::Direction::NODE_INPUT,
												 "Safety Line Valid", "SafetyLineValid");
	safetyStateValidPin = NodePin::createInstance(safetyStateValidSignal, NodePin::Direction::NODE_INPUT,
												  "Safety State Valid", "SafetyStateValid");
	
	resetSafetyFaultPin = NodePin::createInstance(resetSafetySignal, NodePin::Direction::NODE_OUTPUT,
												  "Reset Safety Fault", "ResetSafetyFault");
	stateLedPin = NodePin::createInstance(stateLedSignal, NodePin::Direction::NODE_OUTPUT,
										  "State LED Signal", "StateLEDSignal");
	
	
	faultResetPulseTime = Legato::NumberParameter<double>::createInstance(0.0, "Fault Reset Pulse Time", "FaultResetPulseTime");
	faultResetPulseTime->setUnit(Units::Time::Second);
	unclearedFaultLedBlinkFrequency = Legato::NumberParameter<double>::createInstance(0.0, "Uncleared Fault LED Blink Frequency", "UnclearedFaultLedBlinkFrequency");
	unclearedFaultLedBlinkFrequency->setUnit(Units::Frequency::Hertz);
	
	addNodePin(gpioPin);
	addNodePin(safetyLineValidPin);
	addNodePin(safetyStateValidPin);
	
	addNodePin(resetSafetyFaultPin);
	addNodePin(stateLedPin);
	std::shared_ptr<SafetySignal> thisSafetySignal = downcasted_shared_from_this<SafetySignal>();
	controlWidget = std::make_shared<ControlWidget>(thisSafetySignal);
}

void SafetySignal::inputProcess(){
	
	//check gpio devices for their state
	if(!gpioPin->isConnected() || !safetyStateValidPin->isConnected()) {
		safetyState = State::OFFLINE;
		return;
	}
	else{
		for(auto connectedGpioPin : gpioPin->getConnectedPins()){
			auto gpioDevice = connectedGpioPin->getSharedPointer<GpioInterface>();
			if(gpioDevice->getState() != DeviceState::ENABLED) {
				safetyState = State::OFFLINE;
				return;
			}
		}
	}
	
	safetyStateValidPin->copyConnectedPinValue();
	if(safetyLineValidPin->isConnected()) safetyLineValidPin->copyConnectedPinValue();
	
	if(!*safetyStateValidSignal) {
		if(safetyLineValidPin->isConnected()){
			if(!*safetyLineValidSignal) safetyState = State::EMERGENCY_STOP;
			else safetyState = State::UNCLEARED_SAFETY_FAULT;
		}else{
			safetyState = State::EMERGENCY_STOP;
		}
		//master safety state is in SAFE mode
		//if the safety line is also SAFE, we are in full ESTOP
		//if the safety line is CLEAR, we can CLEAR the master fault
	}
	else{
		if(safetyLineValidPin->isConnected()){
			if(!*safetyLineValidSignal) safetyState = State::EMERGENCY_STOP;
			else safetyState = State::CLEAR;
		}else{
			safetyState = State::CLEAR;
		}
		//master safety state is CLEAR
		//if the safety line is also SAFE
	}

	assert(false || "Timing source should be reimplemented...");
	
	/*
	if(b_shouldResetSafety){
		b_shouldResetSafety = false;
		resetPulseStartTime = Environnement::getTime_nanoseconds();
		b_isResettingFault = true;
	}
	if(b_isResettingFault && Environnement::getTime_nanoseconds() - resetPulseStartTime > faultResetPulseTime->value * 1000'000'000){
		b_isResettingFault = false;
	}
	*/
	
	*resetSafetySignal = b_isResettingFault;
	
	switch(safetyState){
		case State::CLEAR:
            *stateLedSignal = true;
            break;
		case State::OFFLINE:
			*stateLedSignal = false;
			break;
		case State::EMERGENCY_STOP:
            *stateLedSignal = Timing::getBlink(1.0 / unclearedFaultLedBlinkFrequency->getValue());
            break;
		case State::UNCLEARED_SAFETY_FAULT:{
            *stateLedSignal = Timing::getBlink(1.0 / unclearedFaultLedBlinkFrequency->getValue());
			}break;
	}
	
}





bool SafetySignal::onSerialization(){
	bool success = true;
	success &= Node::onSerialization();
	success &= faultResetPulseTime->serializeIntoParent(this);
	success &= unclearedFaultLedBlinkFrequency->serializeIntoParent(this);
	success &= serializeAttribute("WidgetUniqueID", controlWidget->uniqueID);
	return success;
}

bool SafetySignal::onDeserialization(){
	bool success = true;
	success &= Node::onDeserialization();
	success &= faultResetPulseTime->deserializeFromParent(this);
	success &= unclearedFaultLedBlinkFrequency->deserializeFromParent(this);
	success &= deserializeAttribute("WidgetUniqueID", controlWidget->uniqueID);
	return success;
}

