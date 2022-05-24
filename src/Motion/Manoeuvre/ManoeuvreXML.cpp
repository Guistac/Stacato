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
		newTrack->setManoeuvre(manoeuvre);
		manoeuvre->tracks.push_back(newTrack);
		trackXML = trackXML->NextSiblingElement("ParameterTrack");
	}
	
	manoeuvre->validateAllParameterTracks();
	
	return manoeuvre;
}




bool ParameterTrack::save(tinyxml2::XMLElement* xml){
	xml->SetAttribute("Machine", parameter->getMachine()->getName());
	xml->SetAttribute("MachineUniqueID", parameter->getMachine()->getUniqueID());
	xml->SetAttribute("Parameter", parameter->getName());
	xml->SetAttribute("IsGroup", isGroup());
	if(!isGroup()) {
		auto animated = castToAnimated();
		xml->SetAttribute("Type", Enumerator::getSaveString(animated->getType()));
	}
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
	auto loadedTrack = loadType(xml, parameter);
	if(loadedTrack == nullptr) {
		Logger::warn("Failed to load parameter track {} of machine {}", parameterName, machine->getName());
		return nullptr;
	}
	loadedTrack->subscribeToMachineParameter();
	
	return loadedTrack;
}

std::shared_ptr<ParameterTrack> ParameterTrack::loadType(tinyxml2::XMLElement* xml, std::shared_ptr<MachineParameter> parameter){
	using namespace tinyxml2;
	

	bool isGroup = false;
	ManoeuvreType manoeuvreType;
	
	xml->QueryBoolAttribute("IsGroup", &isGroup);
	
	if(!isGroup){
		const char* typeSaveString;
		if(xml->QueryStringAttribute("Type", &typeSaveString) != XML_SUCCESS){
			Logger::warn("could not load type save string");
			return nullptr;
		}
		if(!Enumerator::getEnumeratorFromSaveString(typeSaveString, manoeuvreType)){
			Logger::warn("could not identify track type save string");
			return nullptr;
		}
	}
	
	std::shared_ptr<ParameterTrack> parameterTrack = nullptr;
	if(isGroup) ParameterTrackGroup::load(xml, parameter->castToGroup());
	else{
		switch(manoeuvreType){
			case ManoeuvreType::KEY: parameterTrack = KeyParameterTrack::load(xml, parameter->castToAnimatable()); break;
			case ManoeuvreType::TARGET: parameterTrack = TargetParameterTrack::load(xml, parameter->castToAnimatable()); break;
			case ManoeuvreType::SEQUENCE: parameterTrack = SequenceParameterTrack::load(xml, parameter->castToAnimatable()); break;
		}
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
		
		ManoeuvreType trackType;
		bool isGroup;
						
		const char* parameterName;
		
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

std::shared_ptr<KeyParameterTrack> KeyParameterTrack::load(tinyxml2::XMLElement* xml, std::shared_ptr<AnimatableParameter> parameter){
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
	timeConstraint->save(xml);
	velocityConstraint->save(xml);
	inAcceleration->save(xml);
	outAcceleration->save(xml);
	timeOffset->save(xml);
	return true;
}

std::shared_ptr<TargetParameterTrack> TargetParameterTrack::load(tinyxml2::XMLElement* xml, std::shared_ptr<AnimatableParameter> parameter){
	auto targetParameterTrack = std::make_shared<TargetParameterTrack>(parameter);
	if(!targetParameterTrack->target->load(xml)){
		Logger::warn("could not load attribute target of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!targetParameterTrack->constraintType->load(xml)){
		Logger::warn("could not load attribute constraint Type of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!targetParameterTrack->timeConstraint->load(xml)){
		Logger::warn("could not load attribute time constraint of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!targetParameterTrack->velocityConstraint->load(xml)){
		Logger::warn("could not load attribute velocity constraint of parameter track {}", parameter->getName());
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
	using namespace tinyxml2;
	
	interpolationType->save(xml);
	start->save(xml);
	target->save(xml);
	duration->save(xml);
	timeOffset->save(xml);
	inAcceleration->save(xml);
	outAcceleration->save(xml);
	
	XMLElement* curvesXML = xml->InsertNewChildElement("Curves");
	for(auto& curve : getCurves()){
		XMLElement* curveXML = curvesXML->InsertNewChildElement("Curve");
		for(auto& controlPoint : curve.getPoints()){
			XMLElement* controlPointXML = curveXML->InsertNewChildElement("ControlPoint");
			controlPointXML->SetAttribute("Position", controlPoint->position);
			controlPointXML->SetAttribute("Time", controlPoint->time);
			controlPointXML->SetAttribute("Velocity", controlPoint->velocity);
			controlPointXML->SetAttribute("InAcceleration", controlPoint->inAcceleration);
			controlPointXML->SetAttribute("OutAcceleration", controlPoint->outAcceleration);
		}
	}
	
	return true;
}

std::shared_ptr<SequenceParameterTrack> SequenceParameterTrack::load(tinyxml2::XMLElement* xml, std::shared_ptr<AnimatableParameter> parameter){
	auto track = std::make_shared<SequenceParameterTrack>(parameter);
	if(!track->interpolationType->load(xml)){
		Logger::warn("could not load attribute Interpolation Type of parameter track {}", parameter->getName());
	}
	if(!track->start->load(xml)){
		Logger::warn("could not load attribute Start of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!track->target->load(xml)){
		Logger::warn("could not load attribute End of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!track->duration->load(xml)){
		Logger::warn("could not load attribute Duration of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!track->timeOffset->load(xml)){
		Logger::warn("could not load attribute Time Offset of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!track->inAcceleration->load(xml)){
		Logger::warn("could not load attribute inAcceleration of parameter track {}", parameter->getName());
		return nullptr;
	}
	if(!track->outAcceleration->load(xml)){
		Logger::warn("could not load attribute outAcceleration of parameter track {}", parameter->getName());
		return nullptr;
	}

	
	//load curve points
	//generate curves
	
	
	int curveCount = parameter->getCurveCount();
	auto& curves = track->getCurves();
	auto animatable = track->getAnimatableParameter();
	
	auto startValue = animatable->getParameterValue(track->start);
	auto targetValue = animatable->getParameterValue(track->target);
	std::vector<double> curveStartPositions = animatable->getCurvePositionsFromParameterValue(startValue);
	std::vector<double> curveEndPositions = animatable->getCurvePositionsFromParameterValue(targetValue);
	
	for(int i = 0; i < curveCount; i++){
		auto& curve = curves[i];
		auto& points = curve.getPoints();
		curve.interpolationType = track->interpolationType->value;
		
		auto startPoint = std::make_shared<Motion::ControlPoint>();
		auto targetPoint = std::make_shared<Motion::ControlPoint>();
		
		startPoint->position = curveStartPositions[i];
		startPoint->velocity = 0.0;
		startPoint->inAcceleration = track->inAcceleration->value;
		startPoint->outAcceleration = track->inAcceleration->value;
		startPoint->time = track->timeOffset->value;
		
		targetPoint->position = curveEndPositions[i];
		targetPoint->velocity = 0.0;
		targetPoint->inAcceleration = track->outAcceleration->value;
		targetPoint->outAcceleration = track->outAcceleration->value;
		targetPoint->time = track->timeOffset->value + track->duration->value;
		
		points.push_back(startPoint);
		
		//push intermediary points here
		
		points.push_back(targetPoint);
		
		curve.refresh();
	}
	
	track->duration_seconds = track->timeOffset->value + track->duration->value;
	
	return track;
}
