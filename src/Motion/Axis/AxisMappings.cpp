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



std::shared_ptr<SafetyRule> SafetyRule::loadFromSaveString(std::string saveString, std::shared_ptr<AxisNode> axis){
	static std::vector<SafetyRule*> ruleLibrary = {
		new FeedbackToFeedbackVelocityComparison(nullptr),
		//new OtherRule(nullptr),
		//new CoolRule(nullptr),
		//new WeirdRule(nullptr),
		//new ImportantRule(nullptr)
	};
	for(auto rule : ruleLibrary){
		if(saveString == rule->getSaveString()) return rule->createInstance(axis);
	}
	return nullptr;
}


bool FeedbackToFeedbackVelocityComparison::save(tinyxml2::XMLElement* parent) {
	parent->SetAttribute("Type", getSaveString().c_str());
	parent->SetAttribute("Enabled", b_enabled);
	if(mapping1) parent->SetAttribute("FeedbackMapping1PinSaveName", mapping1->getPin()->saveString);
	if(mapping2) parent->SetAttribute("FeedbackMapping2PinSaveName", mapping2->getPin()->saveString);
	parent->SetAttribute("MaxVelocityDeviation", maxVelocityDeviation);
}

bool FeedbackToFeedbackVelocityComparison::load(tinyxml2::XMLElement* parent) {
	parent->QueryAttribute("Enabled", &b_enabled);
	const char* feedback1PinSaveName;
	if(parent->QueryAttribute("FeedbackMapping1PinSaveName", &feedback1PinSaveName) == tinyxml2::XML_SUCCESS){
		mapping1 = axisNode->findFeedbackMapping(feedback1PinSaveName);
	}
	const char* feedback2PinSaveName;
	if(parent->QueryAttribute("FeedbackMapping2PinSaveName", &feedback2PinSaveName) == tinyxml2::XML_SUCCESS){
		mapping2 = axisNode->findFeedbackMapping(feedback2PinSaveName);
	}
	parent->QueryAttribute("MaxVelocityDeviation", &maxVelocityDeviation);
	return true;
}

void FeedbackToFeedbackVelocityComparison::onAxisInterfaceChange(){
	auto feedbackMappings = axisNode->getFeedbackMappings();
	auto mappingStillExists = [&](std::shared_ptr<FeedbackMapping> testedMapping) -> bool {
		for(auto mapping : feedbackMappings){
			if(mapping == testedMapping) return true;
		}
		return false;
	};
	
	if(!mappingStillExists(mapping1)) mapping1 = nullptr;
	if(!mappingStillExists(mapping2)) mapping2 = nullptr;
}
