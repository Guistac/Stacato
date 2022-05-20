#include <pch.h>

#include "Manoeuvre.h"
#include "ParameterTrack.h"

#include "Machine/Machine.h"
#include "Machine/AnimatableParameter.h"
#include "Environnement/Environnement.h"

#include <tinyxml2.h>


bool Manoeuvre::save(tinyxml2::XMLElement* manoeuvreXML) {
	
	using namespace tinyxml2;
	
	name->save(manoeuvreXML);
	description->save(manoeuvreXML);
	type->save(manoeuvreXML);
	
	for (auto& track : tracks) {
		//tracks which have a group parent are listed in the manoeuvres track vector
		//but they don't get saved in the main manoeuvre
		//instead they are saved by their parent parameter track
		if (track->hasParentGroup()) continue;
		XMLElement* trackXML = manoeuvreXML->InsertNewChildElement("ParameterTrack");
		track->save(trackXML);
	}
	 
	return true;
}

std::shared_ptr<Manoeuvre> Manoeuvre::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	auto manoeuvre = std::make_shared<Manoeuvre>();
	
	manoeuvre->name->load(xml);
	manoeuvre->description->load(xml);
	manoeuvre->type->load(xml);
	
	XMLElement* trackXML = xml->FirstChildElement("ParameterTrack");
	while (trackXML != nullptr) {
		auto newTrack = ParameterTrack::load(trackXML);
		if(newTrack == nullptr) return nullptr;
		manoeuvre->tracks.push_back(newTrack);
		trackXML = trackXML->NextSiblingElement("ParameterTrack");
	}
	
	return manoeuvre;
}




bool ParameterTrack::save(tinyxml2::XMLElement* xml){
	xml->SetAttribute("Machine", parameter->getMachine()->getName());
	xml->SetAttribute("MachineUniqueID", parameter->getMachine()->getUniqueID());
	xml->SetAttribute("Parameter", parameter->getName());
	xml->SetAttribute("Type", Enumerator::getSaveString(getType()));
	return onSave(xml);
}


std::shared_ptr<ParameterTrack> ParameterTrack::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	const char* machineName;
	int machineUniqueID;
	const char* parameterName;
	
	//we need to find the exact parameter object linked with the parameter track
	//to do this we match the machine name, machine unique id and then parse the machines parameters and match the parameter name
	
	if(xml->QueryStringAttribute("Machine", &machineName) != XML_SUCCESS) {
		Logger::warn("could not load machine name");
		return nullptr;
	}
	
	if(xml->QueryIntAttribute("MachineUniqueID", &machineUniqueID) != XML_SUCCESS){
		Logger::warn("Could not load machine unique id");
		return nullptr;
	}
	
	if(xml->QueryStringAttribute("Parameter", &parameterName) != XML_SUCCESS){
		Logger::warn("Could not load parameter name");
		return nullptr;
	}
	
	
	std::shared_ptr<Machine> machine = nullptr;
	for (auto& m : Environnement::getMachines()) {
		if (strcmp(machineName, m->getName()) == 0 && machineUniqueID == m->getUniqueID()) {
			machine = m;
			break;
		}
	}
	if (machine == nullptr) {
		Logger::warn("Could not identify machine {} with unique ID {}", machineName, machineUniqueID);
		return nullptr;
	}

	std::shared_ptr<MachineParameter> parameter = nullptr;
	for (auto& p : machine->parameters) {
		if (strcmp(parameterName, p->getName()) == 0) {
			parameter = p;
			break;
		}
	}
	if (parameter == nullptr) {
		Logger::warn("Could not find parameter named {} in machine {}", parameterName, machine->getName());
		return nullptr;
	}
	
	//once we have the parameter object, we can create and load the parameter track object:
	return loadType(xml, parameter);
}

std::shared_ptr<ParameterTrack> ParameterTrack::loadType(tinyxml2::XMLElement* xml, std::shared_ptr<MachineParameter> parameter){
	using namespace tinyxml2;
	
	Type trackType;
	const char* typeSaveString;
	if(xml->QueryStringAttribute("Type", &typeSaveString) != XML_SUCCESS){
		Logger::warn("could not load type save string");
		return nullptr;
	}
	if(!Enumerator::getEnumeratorFromSaveString(typeSaveString, trackType)){
		Logger::warn("could not identify track type save string");
		return nullptr;
	}
	
	std::shared_ptr<ParameterTrack> parameterTrack = nullptr;
	switch(trackType){
		case Type::GROUP: parameterTrack = ParameterTrackGroup::load(xml, MachineParameter::castToGroup(parameter)); break;
		case Type::KEY: parameterTrack = KeyParameterTrack::load(xml, parameter); break;
		case Type::TARGET: parameterTrack = TargetParameterTrack::load(xml, parameter); break;
		case Type::SEQUENCE: parameterTrack = SequenceParameterTrack::load(xml, parameter); break;
	}
	if(parameterTrack == nullptr){
		Logger::warn("Could not load parameter track");
		return nullptr;
	}
	return parameterTrack;
}











bool ParameterTrackGroup::onSave(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	for(auto childTrack : children){
		XMLElement* childTrackXML = xml->InsertNewChildElement("ParameterTrack");
		childTrack->save(childTrackXML);
	}
	return true;
}

std::shared_ptr<ParameterTrackGroup> ParameterTrackGroup::load(tinyxml2::XMLElement* xml, std::shared_ptr<ParameterGroup> parameterGroup){
	using namespace tinyxml2;
	
	auto parameterTrackGroup = std::make_shared<ParameterTrackGroup>(parameterGroup);
	
	XMLElement* childTrackXML = xml->FirstChildElement("ParameterTrack");
	while(childTrackXML){
		
		//for each child parameter, we need to find the parameter object
		//we parse the children of the parameter group and match the child parameter by name
		
		const char* parameterName;
		Type trackType;
		
		if(xml->QueryStringAttribute("Parameter", &parameterName) != XML_SUCCESS){
			Logger::warn("Could not load child parameter name");
			return nullptr;
		}
		
		std::shared_ptr<MachineParameter> parameter = nullptr;
		for(auto childParameter : parameterGroup->getChildren()){
			if(strcmp(childParameter->getName(), parameterName) == 0){
				parameter = childParameter;
				break;
			}
		}
		if(parameter == nullptr){
			Logger::warn("could not find child parameter {} of parameter group {}", parameterName, parameterGroup->getName());
			return nullptr;
		}
		
		//once we have the parameter name, we can create and load the child parameter track object:
		std::shared_ptr<ParameterTrack> childParameterTrack = ParameterTrack::loadType(xml, parameter);
		if(childParameterTrack == nullptr){
			Logger::warn("could not load child parameter track {}", parameter->getName());
			return nullptr;
		}
		
		//set the grouping dependencies
		childParameterTrack->setParent(parameterTrackGroup);
		parameterTrackGroup->children.push_back(childParameterTrack);
		
		//get the next child parameter
		childTrackXML = xml->NextSiblingElement("ParameterTrack");
	}
	
	
	//here we check if all child parameters of the parameter group have an associated parameter track
	for(auto childParameter : parameterGroup->getChildren()){
		bool b_hasTrack = false;
		for(auto childTrack : parameterTrackGroup->getChildren()){
			if(childTrack->getParameter() == childParameter){
				b_hasTrack = true;
				break;
			}
		}
		if(!b_hasTrack) {
			Logger::warn("Parameter Track Group \'{}\' did not load child parameter track \'{}\'", parameterGroup->getType(), childParameter->getName());
			return nullptr;
		}
	}
	
	return parameterTrackGroup;
}










bool KeyParameterTrack::onSave(tinyxml2::XMLElement* xml){
	target->save(xml);
	return true;
}

std::shared_ptr<KeyParameterTrack> KeyParameterTrack::load(tinyxml2::XMLElement* xml, std::shared_ptr<MachineParameter> parameter){
	auto keyParameterTrack = std::make_shared<KeyParameterTrack>(parameter);
	if(!keyParameterTrack->target->load(xml)){
		Logger::warn("Could not load Target parameter of Parameter track {}", parameter->getName());
		return nullptr;
	}
	return keyParameterTrack;
}












bool TargetParameterTrack::onSave(tinyxml2::XMLElement* xml){
	//save type, constraint, target, ramps
	target->save(xml);
	constraintType->save(xml);
	constraint->save(xml);
	inAcceleration->save(xml);
	outAcceleration->save(xml);
	timeOffset->save(xml);
	return true;
}

std::shared_ptr<TargetParameterTrack> TargetParameterTrack::load(tinyxml2::XMLElement* xml, std::shared_ptr<MachineParameter> parameter){
	auto targetParameterTrack = std::make_shared<TargetParameterTrack>(parameter);
	if(!targetParameterTrack->target->load(xml)){
		Logger::warn("could not load attribute target of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!targetParameterTrack->constraintType->load(xml)){
		Logger::warn("could not load attribute constraint Type of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!targetParameterTrack->constraint->load(xml)){
		Logger::warn("could not load attribute constraint of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!targetParameterTrack->inAcceleration->load(xml)){
		Logger::warn("could not load attribute inAcceleration of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!targetParameterTrack->outAcceleration->load(xml)){
		Logger::warn("could not load attribute outAcceleration of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!targetParameterTrack->timeOffset->load(xml)){
		Logger::warn("could not load attribute timeOffset of parameter track {}", parameter->getName());
		return nullptr;
	}
	return targetParameterTrack;
}










bool SequenceParameterTrack::onSave(tinyxml2::XMLElement* xml){
	start->save(xml);
	target->save(xml);
	return true;
}

std::shared_ptr<SequenceParameterTrack> SequenceParameterTrack::load(tinyxml2::XMLElement* xml, std::shared_ptr<MachineParameter> parameter){
	auto sequenceParameterTrack = std::make_shared<SequenceParameterTrack>(parameter);
	if(!sequenceParameterTrack->start->load(xml)){
		Logger::warn("could not load attribute Start of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!sequenceParameterTrack->target->load(xml)){
		Logger::warn("could not load attribute End of parameter track {}", parameter->getName());
		return nullptr;
	}
	return sequenceParameterTrack;
}
