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
	
	deviceUnitsPerAxisUnits->onEdit();
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
	
	controlModeParameter->onEdit();
	deviceUnitsPerAxisUnits->onEdit();
}



std::shared_ptr<SafetyRule> SafetyRule::loadFromSaveString(std::string saveString, std::shared_ptr<AxisNode> axis){
	static std::vector<SafetyRule*> ruleLibrary = {
		new FeedbackToFeedbackVelocityComparison(nullptr),
		new FeedbackToFeedbackPositionComparison(nullptr)
		//new OtherRule(nullptr)
	};
	for(auto rule : ruleLibrary){
		if(saveString == rule->getSaveString()) return rule->createInstance(axis);
	}
	return nullptr;
}


bool FeedbackToFeedbackVelocityComparison::isRespected() {
	if(mapping1 == nullptr || !mapping1->isFeedbackConnected()) return false;
	if(mapping2 == nullptr || !mapping2->isFeedbackConnected()) return false;
	auto feedbackDevice1 = mapping1->getFeedbackInterface();
	auto feedbackDevice2 = mapping2->getFeedbackInterface();
	if(!feedbackDevice1->isReady()) return false;
	if(!feedbackDevice2->isReady()) return false;
	double feedback1velocity = feedbackDevice1->getVelocity() / mapping1->deviceUnitsPerAxisUnits->value;
	double feedback2velocity = feedbackDevice2->getVelocity() / mapping2->deviceUnitsPerAxisUnits->value;
	double velocityDeviation = std::abs(feedback1velocity - feedback2velocity);
	if(velocityDeviation > maxVelocityDeviation) {
		Logger::warn("{} Velocity Safety Rule Exceeded", axisNode->getName());
		Logger::warn("Feedback1: {}   Feedback2: {}", feedback1velocity, feedback2velocity);
		return false;
	}
	return true;
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



bool FeedbackToFeedbackPositionComparison::isRespected() {
	if(mapping1 == nullptr || !mapping1->isFeedbackConnected()) return false;
	if(mapping2 == nullptr || !mapping2->isFeedbackConnected()) return false;
	auto feedbackDevice1 = mapping1->getFeedbackInterface();
	auto feedbackDevice2 = mapping2->getFeedbackInterface();
	if(!feedbackDevice1->isReady()) return false;
	if(!feedbackDevice2->isReady()) return false;
	double feedback1position;
	double feedback2position;
	
	if(feedbackDevice1->getType() == DeviceInterface::Type::ACTUATOR){
		auto actuatorMapping = std::static_pointer_cast<ActuatorMapping>(mapping1);
		feedback1position = (feedbackDevice1->getPosition() - actuatorMapping->actuatorPositionOffset) / mapping1->deviceUnitsPerAxisUnits->value;
	}else feedback1position = feedbackDevice1->getPosition() / mapping1->deviceUnitsPerAxisUnits->value;
	
	if(feedbackDevice2->getType() == DeviceInterface::Type::ACTUATOR){
		auto actuatorMapping = std::static_pointer_cast<ActuatorMapping>(mapping2);
		feedback2position = (feedbackDevice2->getPosition() - actuatorMapping->actuatorPositionOffset) / mapping2->deviceUnitsPerAxisUnits->value;
	}else feedback2position = feedbackDevice2->getPosition() / mapping2->deviceUnitsPerAxisUnits->value;
	
	
	double positionDeviation = std::abs(feedback1position - feedback2position);
	if(positionDeviation > maxPositionDeviation) {
		Logger::warn("{} Position Safety Rule Exceeded", axisNode->getName());
		Logger::warn("Feedback1: {}   Feedback2: {}", feedback1position, feedback2position);
		return false;
	}
	return true;
}

bool FeedbackToFeedbackPositionComparison::save(tinyxml2::XMLElement* parent) {
	parent->SetAttribute("Type", getSaveString().c_str());
	parent->SetAttribute("Enabled", b_enabled);
	if(mapping1) parent->SetAttribute("FeedbackMapping1PinSaveName", mapping1->getPin()->saveString);
	if(mapping2) parent->SetAttribute("FeedbackMapping2PinSaveName", mapping2->getPin()->saveString);
	parent->SetAttribute("MaxPositionDeviation", maxPositionDeviation);
}

bool FeedbackToFeedbackPositionComparison::load(tinyxml2::XMLElement* parent) {
	parent->QueryAttribute("Enabled", &b_enabled);
	const char* feedback1PinSaveName;
	if(parent->QueryAttribute("FeedbackMapping1PinSaveName", &feedback1PinSaveName) == tinyxml2::XML_SUCCESS){
		mapping1 = axisNode->findFeedbackMapping(feedback1PinSaveName);
	}
	const char* feedback2PinSaveName;
	if(parent->QueryAttribute("FeedbackMapping2PinSaveName", &feedback2PinSaveName) == tinyxml2::XML_SUCCESS){
		mapping2 = axisNode->findFeedbackMapping(feedback2PinSaveName);
	}
	parent->QueryAttribute("MaxPositionDeviation", &maxPositionDeviation);
	return true;
}

void FeedbackToFeedbackPositionComparison::onAxisInterfaceChange(){
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

