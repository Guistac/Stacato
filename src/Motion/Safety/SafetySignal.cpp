#include <pch.h>

#include "SafetySignal.h"
#include "Motion/SubDevice.h"

#include "Environnement/Environnement.h"

void SafetySignal::initialize(){
	addNodePin(gpioPin);
	addNodePin(safetyLineStatePin);
	addNodePin(safetyValidPin);
	
	addNodePin(resetSafetyFaultPin);
	addNodePin(stateLedPin);
	std::shared_ptr<SafetySignal> thisSafetySignal = std::static_pointer_cast<SafetySignal>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisSafetySignal);
}

void SafetySignal::inputProcess(){
	safetyLineStatePin->copyConnectedPinValue();
	safetyValidPin->copyConnectedPinValue();
}

void SafetySignal::outputProcess(){}





bool SafetySignal::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;

	XMLElement* controlWidgetXML = xml->InsertNewChildElement("ControlWidget");
	controlWidgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	
	return true;
}

bool SafetySignal::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* controlWidgetXML;
	if(!loadXMLElement("ControlWidget", xml, controlWidgetXML)) return false;
	if(controlWidgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS){
		Logger::warn("could not load dead mans switch control widget unique id");
		return false;
	}
	
	return true;
	
}

