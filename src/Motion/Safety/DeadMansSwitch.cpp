#include <pch.h>

#include "DeadMansSwitch.h"

void DeadMansSwitch::initialize(){
	addNodePin(gpioDevicePin);
	addNodePin(switchPressedPin);
	addNodePin(switchConnectedPin);
	addNodePin(switchLedPin);
	std::shared_ptr<DeadMansSwitch> thisDeadMansSwitch = std::static_pointer_cast<DeadMansSwitch>(shared_from_this());
	deadMansSwitchLink->assignData(thisDeadMansSwitch);
	addNodePin(deadMansSwitchLink);
	
	controlWidget = std::make_shared<ControlWidget>(thisDeadMansSwitch, getName());
}

void DeadMansSwitch::inputProcess(){
	
	switchPressedPin->copyConnectedPinValue();
	switchConnectedPin->copyConnectedPinValue();
	
	*b_switchLed = *b_switchPressed;
}

void DeadMansSwitch::outputProcess(){
	//Logger::critical("output process not defined for dead mans switch");
	//abort();
}

bool DeadMansSwitch::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* controlWidgetXML = xml->InsertNewChildElement("ControlWidget");
	controlWidgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	return true;
}

bool DeadMansSwitch::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* controlWidgetXML = xml->FirstChildElement("ControlWidget");
	if(!controlWidgetXML) {
		Logger::warn("could not find control widget attribute of dead mans switch");
		return false;
	}
	
	if(controlWidgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS){
		Logger::warn("could not load dead mans switch control widget unique id");
		return false;
	}
	
	return true;
	
}

