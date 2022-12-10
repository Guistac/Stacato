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
	
	if(*brakeOpenStatus_Signal && *brakeClosedStatus_Signal) state = State::ERROR;
	else if(*brakeOpenStatus_Signal) state = State::OPEN;
	else if(*brakeClosedControl_Signal) state = State::CLOSED;
	else state = State::NOT_CLOSED;
	
	switch(target){
		case Target::OPEN:
			*brakeOpenStatus_Signal = true;
			*brakeClosedStatus_Signal = false;
			break;
		case Target::CLOSED:
			*brakeOpenStatus_Signal = false;
			*brakeClosedStatus_Signal = true;
			break;
		case Target::STOPPED:
			*brakeOpenStatus_Signal = false;
			*brakeClosedStatus_Signal = false;
	}
	
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

