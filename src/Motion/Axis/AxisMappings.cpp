#include "AxisMappings.h"

#include "AxisNode.h"

#include <tinyxml2.h>


FeedbackMapping::FeedbackMapping(std::shared_ptr<AxisNode> axis) : axisNode(axis){
	deviceUnitsPerAxisUnits->addEditCallback([this](){ axisNode->updateAxisConfiguration(); });
}


ActuatorMapping::ActuatorMapping(std::shared_ptr<AxisNode> axis) : FeedbackMapping(axis){
	controlModeParameter->addEditCallback([this](){ controlMode = (ActuatorControlMode)controlModeParameter->value; });
	controlModeParameter->onEdit();
	controlModeParameter->addEditCallback([this](){ axisNode->updateControlMode(); });
}

bool FeedbackMapping::save(tinyxml2::XMLElement* parent){
	parent->SetAttribute("PinSaveName", feedbackPin->saveString);
	deviceUnitsPerAxisUnits->save(parent);
}
bool FeedbackMapping::load(tinyxml2::XMLElement* parent){
	const char* pinSaveString;
	parent->QueryAttribute("PinSaveName", &pinSaveString);
	strcpy(feedbackPin->saveString, pinSaveString);
	deviceUnitsPerAxisUnits->load(parent);
}



bool ActuatorMapping::save(tinyxml2::XMLElement* parent){
	parent->SetAttribute("PinSaveName", actuatorPin->saveString);
	controlModeParameter->save(parent);
	deviceUnitsPerAxisUnits->save(parent);
	parent->SetAttribute("PositionOffset", actuatorPositionOffset);
}
bool ActuatorMapping::load(tinyxml2::XMLElement* parent){
	const char* pinSaveString;
	parent->QueryAttribute("PinSaveName", &pinSaveString);
	strcpy(actuatorPin->saveString, pinSaveString);
	controlModeParameter->load(parent);
	deviceUnitsPerAxisUnits->load(parent);
	parent->QueryAttribute("PositionOffset", &actuatorPositionOffset);
}
