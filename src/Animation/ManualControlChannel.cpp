#include <pch.h>
#include "ManualControlChannel.h"

#include "Animatable.h"
#include "Machine/Machine.h"

#include <tinyxml2.h>

bool AnimatableMapping::save(tinyxml2::XMLElement* xml){
	multiplier->save(xml);
	
	xml->SetAttribute("Machine", animatable->getMachine()->getName());
	xml->SetAttribute("MachineUniqueID", animatable->getMachine()->getUniqueID());
	xml->SetAttribute("Animatable", animatable->getName());
	
	return false;
}

bool AnimatableMapping::load(tinyxml2::XMLElement* xml){
	multiplier->load(xml);
	
	using namespace tinyxml2;
	
	//we need to find the exact parameter object linked with the parameter track
	//to do this we match the machine name, machine unique id and then parse the machines parameters and match the parameter name
	const char* machineName;
	int machineUniqueID;
	const char* animatableName;
	if(xml->QueryStringAttribute("Machine", &machineName) != XML_SUCCESS) return Logger::warn("could not load machine name");
	if(xml->QueryIntAttribute("MachineUniqueID", &machineUniqueID) != XML_SUCCESS) return Logger::warn("Could not load machine unique id");
	if(xml->QueryStringAttribute("Animatable", &animatableName) != XML_SUCCESS) return Logger::warn("Could not load parameter name");
	
	std::shared_ptr<Machine> machine = nullptr;
	for (auto& m : Environnement::getMachines()) {
		if (strcmp(machineName, m->getName()) == 0 && machineUniqueID == m->getUniqueID()) { machine = m; break; }
	}
	if (machine == nullptr) return Logger::warn("Could not identify machine {} with unique ID {}", machineName, machineUniqueID);

	for (auto& a : machine->getAnimatables()) {
		if (strcmp(animatableName, a->getName()) == 0) { animatable = a; break; }
	}
	if (animatable == nullptr) return Logger::warn("Could not find animatable named {} in machine {}", animatableName, machine->getName());

	return true;
}

void ChannelPreset::addAnimatable(std::shared_ptr<Animatable> animatable){
	if(hasAnimatable(animatable)) return;
	std::lock_guard<std::mutex> lock(mutex);
	auto animatableMapping = std::make_shared<AnimatableMapping>();
	animatableMapping->animatable = animatable;
	animatableMappings.push_back(animatableMapping);
}

bool ChannelPreset::hasAnimatable(std::shared_ptr<Animatable> animatable){
	std::lock_guard<std::mutex> lock(mutex);
	for(auto mapping : animatableMappings){
		if(mapping->animatable == animatable) return true;
	}
	return false;
}

void ChannelPreset::removeAnimatable(std::shared_ptr<Animatable> animatable){
	std::lock_guard<std::mutex> lock(mutex);
	for(int i = 0; i < animatableMappings.size(); i++){
		if(animatableMappings[i]->animatable == animatable){
			animatableMappings[i]->animatable->setManualControlTarget(0.0);
			animatableMappings.erase(animatableMappings.begin() + i);
			break;
		}
	}
}

std::vector<std::shared_ptr<AnimatableMapping>> ChannelPreset::getMappings(){
	std::lock_guard<std::mutex> lock(mutex);
	return animatableMappings;
}

bool ChannelPreset::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	nameParameter->save(xml);
	
	XMLElement* animatableMappingsXML = xml->InsertNewChildElement("AnimatableMappings");
	for(auto mapping : animatableMappings){
		XMLElement* mappingXML = animatableMappingsXML->InsertNewChildElement("AnimatableMapping");
		mapping->save(mappingXML);
	}
	
	return true;
}

bool ChannelPreset::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	nameParameter->load(xml);
	
	XMLElement* animatableMappingsXML = xml->FirstChildElement("AnimatableMappings");
	XMLElement* mappingXML = animatableMappingsXML->FirstChildElement("AnimatableMapping");
	while(mappingXML){
		auto mapping = std::make_shared<AnimatableMapping>();
		mapping->load(mappingXML);
		animatableMappings.push_back(mapping);
		mappingXML = mappingXML->NextSiblingElement("AnimatableMapping");
	}
	
	return true;
}

void ManualControlChannel::createChannelPreset(){
	auto preset = std::make_shared<ChannelPreset>();
	channelPresets.push_back(preset);
	std::string name = preset->nameParameter->value + " " + std::to_string(channelPresets.size());
	preset->nameParameter->overwrite(name);
	setActiveChannelPreset(preset);
}

void ManualControlChannel::removeChannelPreset(std::shared_ptr<ChannelPreset> preset){
	for(int i = 0; i < channelPresets.size(); i++){
		if(channelPresets[i] == preset){
			for(auto mapping : preset->getMappings()) mapping->animatable->setManualControlTarget(0.0);
			channelPresets.erase(channelPresets.begin() + i);
			if(preset == activeChannelPreset) setActiveChannelPreset(nullptr);
			break;
		}
	}
}

void ManualControlChannel::setActiveChannelPreset(std::shared_ptr<ChannelPreset> preset){
	//set all previous mappings to control value 0
	setControlValue(0.0);
	//then set the new preset
	activeChannelPreset = preset;
}

void ManualControlChannel::setControlValue(float controlValue){
	controlSliderValue = std::clamp(controlValue, -1.0f, 1.0f);
	if(activeChannelPreset == nullptr) return;
	for(auto mapping : activeChannelPreset->getMappings()){
		mapping->animatable->setManualControlTarget(controlSliderValue * mapping->multiplier->value);
	}
}

bool ManualControlChannel::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* channelMappingsXML = xml->InsertNewChildElement("ChannelPresets");
	for(auto mapping : channelPresets){
		XMLElement* channelMappingXML = channelMappingsXML->InsertNewChildElement("ChannelPreset");
		mapping->save(channelMappingXML);
	}
	if(activeChannelPreset) xml->SetAttribute("ActivePresetName", activeChannelPreset->nameParameter->value.c_str());
	return true;
}

bool ManualControlChannel::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* channelPresetsXML = xml->FirstChildElement("ChannelPresets");
	XMLElement* channelPresetXML = channelPresetsXML->FirstChildElement("ChannelPreset");
	while(channelPresetXML){
		auto preset = std::make_shared<ChannelPreset>();
		preset->load(channelPresetXML);
		channelPresets.push_back(preset);
		channelPresetXML = channelPresetXML->NextSiblingElement("ChannelPreset");
	}
	const char* activePresetName;
	if(xml->QueryStringAttribute("ActivePresetName", &activePresetName) == XML_SUCCESS){
		for(auto preset : channelPresets){
			if(activePresetName == preset->nameParameter->value){
				setActiveChannelPreset(preset);
				break;
			}
		}
	}
	return true;
}
