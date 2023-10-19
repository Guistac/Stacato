#include <pch.h>

#include "MultipointHoist.h"

#include "Motion/Axis/AxisNode.h"
#include "Animation/Animatable.h"
#include "Animation/Animation.h"

#include "Environnement/NodeGraph/NodePin.h"

#include "Animation/Manoeuvre.h"

#include <tinyxml2.h>

#include "Environnement/Environnement.h"

#include "Fieldbus/EtherCatFieldbus.h"

#include "Motion/Safety/DeadMansSwitch.h"

void MultipointHoist::initialize() {
	auto thisMachine = std::static_pointer_cast<MultipointHoist>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisMachine);
}

void MultipointHoist::onPinUpdate(std::shared_ptr<NodePin> pin){
	for(auto axisMapping : axisMappings){
		if(pin == axisMapping->axisPin){
			auto axisInterface = pin->getConnectedPin()->getSharedPointer<AxisInterface>();
			axisMapping->animatablePosition->setName(axisInterface->getName());
			axisMapping->updateAnimatableParameters();
			break;
		}
	}
}

void MultipointHoist::onPinConnection(std::shared_ptr<NodePin> pin){
	for(auto axisMapping : axisMappings){
		if(pin == axisMapping->axisPin){
			auto axisInterface = pin->getConnectedPin()->getSharedPointer<AxisInterface>();
			axisMapping->animatablePosition->setName(axisInterface->getName());
			axisMapping->updateAnimatableParameters();
			break;
		}
	}
}

void MultipointHoist::onPinDisconnection(std::shared_ptr<NodePin> pin){
	for(auto axisMapping : axisMappings){
		if(pin == axisMapping->axisPin){
			axisMapping->animatablePosition->setName(pin->displayString);
			axisMapping->updateAnimatableParameters();
			break;
		}
	}
}

bool MultipointHoist::isHardwareReady() {
	for(auto mapping : axisMappings){
		if(!mapping->isAxisConnected()) return false;
		auto axisInterface = mapping->getAxis();
		if(!axisInterface->isReady()) return false;
	}
	return true;
}

bool MultipointHoist::isMoving() { return false; }

void MultipointHoist::enableHardware() {
	for(auto mapping : axisMappings){
		if(!mapping->isAxisConnected()) continue;
		mapping->getAxis()->enable();
	}
}

void MultipointHoist::disableHardware() {
	for(auto mapping : axisMappings){
		if(!mapping->isAxisConnected()) continue;
		mapping->getAxis()->disable();
	}
}

void MultipointHoist::inputProcess() {
	
	DeviceState lowestState = DeviceState::ENABLED;
	b_emergencyStopActive = false;
	for(auto mapping : axisMappings){
		if(!mapping->isAxisConnected()) {
			lowestState = DeviceState::OFFLINE;
			break;
		}
		auto axis = mapping->getAxis();
		if(axis->isEmergencyStopActive()) {
			b_emergencyStopActive = true;
			lowestState = DeviceState::NOT_READY;
			break;
		}
		else if(axis->getState() < lowestState) lowestState = axis->getState();
	}
	state = lowestState;
	
	Animatable::State animatableState;
	if(!b_enableGroupSurveillance) animatableState = Animatable::State::NOT_READY;
	else{
		switch(lowestState){
			case DeviceState::OFFLINE:
				animatableState = Animatable::State::OFFLINE;
				break;
			case DeviceState::NOT_READY:
			case DeviceState::READY:
			case DeviceState::DISABLING:
			case DeviceState::ENABLING:
				animatableState = Animatable::State::NOT_READY;
				break;
			case DeviceState::ENABLED:
				animatableState = Animatable::State::READY;
				break;
		}
	}
	for(auto mapping : axisMappings) mapping->animatablePosition->state = animatableState;
	
	for(auto mapping : axisMappings) mapping->updateRealAnimatableValues();
	
	//update distance constraints
}

void MultipointHoist::outputProcess(){

	//handle group surveillance
	if(b_enableGroupSurveillance){
		bool b_anyAxisEnabled = false;
		bool b_anyAxisDisabled = false;
		for(auto mapping : axisMappings){
			if(!mapping->isAxisConnected()){
				b_anyAxisDisabled = true;
				continue;
			}
			DeviceState axisState = mapping->getAxis()->getState();
			if(axisState == DeviceState::ENABLED || axisState == DeviceState::ENABLING) b_anyAxisEnabled = true;
			else b_anyAxisDisabled = true;
		}
		if(b_anyAxisEnabled && b_anyAxisDisabled){
			disable();
			Logger::warn("[{}] Group surveillance disabled all axes", getName());
			for(auto mapping : axisMappings){
				Logger::info("	{} was {}", mapping->getAxis()->getName(), deviceStateToString(mapping->getAxis()->getState()));
			}
		}
	}
	
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
	for(auto mapping : axisMappings) mapping->updateAxisCommand(profileTime_seconds, profileDeltaTime_seconds);
}



void MultipointHoist::onEnableHardware() {}
void MultipointHoist::onDisableHardware() {}
bool MultipointHoist::isSimulationReady(){}
void MultipointHoist::onEnableSimulation() {}
void MultipointHoist::onDisableSimulation() {}
std::string MultipointHoist::getStatusString(){ return "No Status..."; }
void MultipointHoist::simulateInputProcess() {}
void MultipointHoist::simulateOutputProcess(){}




//========= ANIMATABLE OWNER ==========

void MultipointHoist::fillAnimationDefaults(std::shared_ptr<Animation> animation){
	for(auto mapping : axisMappings){
		auto animatable = mapping->animatablePosition;
		if(animation->getAnimatable() == animatable){
			switch(animation->getType()){
				case ManoeuvreType::KEY:
					animation->toKey()->captureTarget();
					break;
				case ManoeuvreType::TARGET:
					animation->toTarget()->captureTarget();
					animation->toTarget()->inAcceleration->overwrite(animatable->accelerationLimit);
					animation->toTarget()->outAcceleration->overwrite(animatable->accelerationLimit);
					animation->toTarget()->velocityConstraint->overwrite(0.0);
					animation->toTarget()->timeConstraint->overwrite(0.0);
					animation->toTarget()->constraintType->overwrite(TargetAnimation::Constraint::TIME);
					break;
				case ManoeuvreType::SEQUENCE:
					animation->toSequence()->captureStart();
					animation->toSequence()->captureTarget();
					animation->toSequence()->duration->overwrite(0.0);
					animation->toSequence()->inAcceleration->overwrite(animatable->accelerationLimit);
					animation->toSequence()->outAcceleration->overwrite(animatable->accelerationLimit);
					break;
			}
			return;
		}
	}
}



void MultipointHoist::getDevices(std::vector<std::shared_ptr<Device>>& output) {}






void MultipointHoist::setMasterVelocityTarget(double velocityTarget){
	for(auto mapping : axisMappings){
		auto animatable = mapping->animatablePosition;
		animatable->setManualControlTarget(velocityTarget);
	}
}






void MultipointHoist::addAxisMapping(){
	auto newMapping = std::make_shared<AxisMapping>();
	snprintf(newMapping->axisPin->displayString, 64, "Axis %i", (int)axisMappings.size());
	snprintf(newMapping->axisPin->saveString, 64, "Axis%i", (int)axisMappings.size());
	addAxisMapping(newMapping);
}

void MultipointHoist::addAxisMapping(std::shared_ptr<AxisMapping> mapping){
	axisMappings.push_back(mapping);
	addNodePin(mapping->axisPin);
	addAnimatable(mapping->animatablePosition);
}

void MultipointHoist::removeAxisMapping(std::shared_ptr<AxisMapping> mapping){
	for(int i = 0; i < axisMappings.size(); i++){
		if(axisMappings[i] == mapping){
			axisMappings.erase(axisMappings.begin() + i);
			break;
		}
	}
	removeNodePin(mapping->axisPin);
	removeAnimatable(mapping->animatablePosition);
}




bool MultipointHoist::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	XMLElement* axisMappingsXML = xml->InsertNewChildElement("AxisMappings");
	for(auto axisMapping : axisMappings){
		tinyxml2::XMLElement* mappingXML = axisMappingsXML->InsertNewChildElement("AxisMapping");
		axisMapping->save(mappingXML);
	}
	
	xml->SetAttribute("MaxDistanceBetweenAxes", maxDistanceBetweenAxes);
	xml->SetAttribute("EnableTwoAxisDistanceConstraint", b_enableTwoAxisDistanceConstraint);
	xml->SetAttribute("MaxDistanceBetweenTwoAxes", maxDistanceBetweenAxes);
	
	XMLElement* widgetXML = xml->InsertNewChildElement("ControWidget");
	widgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	
	return true;
}

bool MultipointHoist::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	XMLElement* axisMappingsXML = xml->FirstChildElement("AxisMappings");
	if(!axisMappingsXML) return false;
	XMLElement* mappingXML = axisMappingsXML->FirstChildElement("AxisMapping");
	while(mappingXML){
		auto newMapping = std::make_shared<AxisMapping>();
		newMapping->load(mappingXML);
		addAxisMapping(newMapping);
		mappingXML = mappingXML->NextSiblingElement("AxisMapping");
	}
	
	xml->QueryAttribute("MaxDistanceBetweenAxes", &maxDistanceBetweenAxes);
	xml->QueryAttribute("EnableTwoAxisDistanceConstraint", &b_enableTwoAxisDistanceConstraint);
	xml->QueryAttribute("MaxDistanceBetweenTwoAxes", &maxDistanceBetweenAxes);
	
	XMLElement* widgetXML = xml->FirstChildElement("ControWidget");
	if(widgetXML == nullptr) return Logger::warn("Could not find Control Widget Attribute");
	if(widgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS) return Logger::warn("Could not find machine control widget uid attribute");
	
	return true;
}
