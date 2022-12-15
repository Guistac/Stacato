#include <pch.h>

#include "Brake.h"

#include "Motion/SubDevice.h"

//#include "Environnement/Environnement.h"

void Brake::initialize(){
	
	addNodePin(gpio_Pin);
	addNodePin(brakeOpenStatus_Pin);
	addNodePin(brakeClosedStatus_Pin);
	addNodePin(brakeOpenControl_Pin);
	addNodePin(brakeClosedControl_Pin);
	
	controlWidget = std::make_shared<ControlWidget>(std::static_pointer_cast<Brake>(shared_from_this()));
}

void Brake::inputProcess(){
	
	//check gpio devices for their state
	if(!gpio_Pin->isConnected()) {
		state = State::OFFLINE;
		return;
	}
	else{
		for(auto connectedGpioPin : gpio_Pin->getConnectedPins()){
			auto gpioDevice = connectedGpioPin->getSharedPointer<GpioDevice>();
			if(gpioDevice->getState() != MotionState::ENABLED) {
				state = State::OFFLINE;
				return;
			}
		}
	}
	
	if(brakeOpenStatus_Pin->isConnected()) brakeOpenStatus_Pin->copyConnectedPinValue();
	if(brakeClosedStatus_Pin->isConnected()) brakeClosedStatus_Pin->copyConnectedPinValue();
	
	bool b_isOpen = *brakeOpenStatus_Signal;
	bool b_isClosed = *brakeClosedStatus_Signal;
	
	if(b_isClosed && b_isOpen) state = State::ERROR;
	else if(b_isOpen) state = State::OPEN;
	else if(b_isClosed) state = State::CLOSED;
	else state = State::NOT_CLOSED;
	
	bool b_open = false;
	bool b_close = false;
	
	switch(target){
		case Target::OPEN:
			b_open = true;
			b_close = false;
			break;
		case Target::CLOSED:
			b_open = false;
			b_close = true;
			break;
		case Target::STOPPED:
			b_open = false;
			b_close = false;
			break;
	}
	*brakeOpenControl_Signal = b_open;
	*brakeClosedControl_Signal = b_close;
	
}





bool Brake::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* controlWidgetXML = xml->InsertNewChildElement("ControlWidget");
	controlWidgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	
	return true;
}

bool Brake::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* controlWidgetXML;
	if(!loadXMLElement("ControlWidget", xml, controlWidgetXML)) return false;
	if(controlWidgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS){
		Logger::warn("could not load dead mans switch control widget unique id");
		return false;
	}
	
	return true;
}

