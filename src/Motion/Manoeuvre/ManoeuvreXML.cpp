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
	
	for (auto& animation : animations) {
		//tracks which have a group parent are listed in the manoeuvres track vector
		//but they don't get saved in the main manoeuvre
		//instead they are saved by their parent parameter track
		if (animation->hasParentComposite()) continue;
		XMLElement* animationXML = manoeuvreXML->InsertNewChildElement("Animation");
		animation->save(animationXML);
	}
	 
	return true;
}

std::shared_ptr<Manoeuvre> Manoeuvre::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	auto manoeuvre = std::make_shared<Manoeuvre>();
	
	manoeuvre->name->load(xml);
	manoeuvre->description->load(xml);
	manoeuvre->type->load(xml);
	
	XMLElement* animationXML = xml->FirstChildElement("Animation");
	while (animationXML != nullptr) {
		auto newAnimation = Animation::load(animationXML);
		if(newAnimation == nullptr) return nullptr;
		newAnimation->setManoeuvre(manoeuvre);
		manoeuvre->animations.push_back(newAnimation);
		animationXML = animationXML->NextSiblingElement("Animation");
	}
	
	manoeuvre->validateAllParameterTracks();
	
	return manoeuvre;
}




bool Animation::save(tinyxml2::XMLElement* xml){
	xml->SetAttribute("Machine", animatable->getMachine()->getName());
	xml->SetAttribute("MachineUniqueID", animatable->getMachine()->getUniqueID());
	xml->SetAttribute("Parameter", animatable->getName());
	xml->SetAttribute("IsComposite", isComposite());
	if(!isComposite()) xml->SetAttribute("Type", Enumerator::getSaveString(getType()));
	return onSave(xml);
}


std::shared_ptr<Animation> Animation::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	const char* machineName;
	int machineUniqueID;
	const char* animatableName;
	
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
	
	if(xml->QueryStringAttribute("Parameter", &animatableName) != XML_SUCCESS){
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

	std::shared_ptr<Animatable> animatable = nullptr;
	for (auto& a : machine->animatables) {
		if (strcmp(animatableName, a->getName()) == 0) {
			animatable = a;
			break;
		}
	}
	if (animatable == nullptr) {
		Logger::warn("Could not find animatable named {} in machine {}", animatableName, machine->getName());
		return nullptr;
	}
	
	//once we have the parameter object, we can create and load the parameter track object:
	auto loadedTrack = loadType(xml, animatable);
	if(loadedTrack == nullptr) {
		Logger::warn("Failed to load parameter track {} of machine {}", animatableName, machine->getName());
		return nullptr;
	}
	loadedTrack->subscribeToMachineParameter();
	
	return loadedTrack;
}



std::shared_ptr<Animation> Animation::loadType(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable){
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
	
	std::shared_ptr<Animation> animation = nullptr;
	if(isGroup) AnimationComposite::load(xml, animatable->toComposite());
	else{
		switch(manoeuvreType){
			case ManoeuvreType::KEY: animation = AnimationKey::load(xml, animatable); break;
			case ManoeuvreType::TARGET: animation = TargetAnimation::load(xml, animatable); break;
			case ManoeuvreType::SEQUENCE: animation = SequenceAnimation::load(xml, animatable); break;
		}
	}
	if(animation == nullptr){
		Logger::warn("Could not load parameter track");
		return nullptr;
	}
	return animation;
}











bool AnimationComposite::onSave(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	for(auto childTrack : children){
		XMLElement* childTrackXML = xml->InsertNewChildElement("ParameterTrack");
		childTrack->save(childTrackXML);
	}
	return true;
}

std::shared_ptr<AnimationComposite> AnimationComposite::load(tinyxml2::XMLElement* xml, std::shared_ptr<AnimatableComposite> animatableComposite){
	using namespace tinyxml2;
	
	auto animationComposite = std::make_shared<AnimationComposite>(animatableComposite);
	
	XMLElement* childTrackXML = xml->FirstChildElement("ParameterTrack");
	while(childTrackXML){
		
		//for each child parameter, we need to find the parameter object
		//we parse the children of the parameter group and match the child parameter by name
		
		ManoeuvreType trackType;
		bool isGroup;
						
		const char* animatableName;
		
		if(xml->QueryStringAttribute("Animatable", &animatableName) != XML_SUCCESS){
			Logger::warn("Could not load child animatable name");
			return nullptr;
		}
		
		std::shared_ptr<Animatable> animatable = nullptr;
		for(auto childAnimatable : animatableComposite->getChildren()){
			if(strcmp(childAnimatable->getName(), animatableName) == 0){
				animatable = childAnimatable;
				break;
			}
		}
		if(animatable == nullptr){
			Logger::warn("could not find child animatable {} of animatable composite {}", animatableName, animatableComposite->getName());
			return nullptr;
		}
		
		//once we have the parameter name, we can create and load the child parameter track object:
		std::shared_ptr<Animation> childAnimation = Animation::loadType(xml, animatable);
		if(childAnimation == nullptr){
			Logger::warn("could not load child animation {}", animatable->getName());
			return nullptr;
		}
		
		//set the grouping dependencies
		childAnimation->setParentComposite(animationComposite);
		animationComposite->children.push_back(childAnimation);
		
		//get the next child parameter
		childTrackXML = xml->NextSiblingElement("ParameterTrack");
	}
	
	
	//here we check if all child parameters of the parameter group have an associated parameter track
	for(auto childAnimatable : animatableComposite->getChildren()){
		bool b_hasAnimation = false;
		for(auto childAnimation : animationComposite->getChildren()){
			if(childAnimation->getAnimatable() == childAnimatable){
				b_hasAnimation = true;
				break;
			}
		}
		if(!b_hasAnimation) {
			Logger::warn("Animation Composite \'{}\' did not load child animation \'{}\'", animatableComposite->getType(), childAnimatable->getName());
			return nullptr;
		}
	}
	
	return animationComposite;
}










bool AnimationKey::onSave(tinyxml2::XMLElement* xml){
	target->save(xml);
	return true;
}

std::shared_ptr<AnimationKey> AnimationKey::load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable){
	auto animationKey = std::make_shared<AnimationKey>(animatable);
	if(!animationKey->target->load(xml)){
		Logger::warn("Could not load Target parameter of Parameter track {}", animatable->getName());
		return nullptr;
	}
	return animationKey;
}












bool TargetAnimation::onSave(tinyxml2::XMLElement* xml){
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

std::shared_ptr<TargetAnimation> TargetAnimation::load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable){
	auto targetAnimation = std::make_shared<TargetAnimation>(animatable);
	if(!targetAnimation->target->load(xml)){
		Logger::warn("could not load attribute target of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->constraintType->load(xml)){
		Logger::warn("could not load attribute constraint Type of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->timeConstraint->load(xml)){
		Logger::warn("could not load attribute time constraint of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->velocityConstraint->load(xml)){
		Logger::warn("could not load attribute velocity constraint of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->inAcceleration->load(xml)){
		Logger::warn("could not load attribute inAcceleration of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->outAcceleration->load(xml)){
		Logger::warn("could not load attribute outAcceleration of animation {}", animatable->getName());
		return nullptr;
	}
	if(!targetAnimation->timeOffset->load(xml)){
		Logger::warn("could not load attribute timeOffset of animation {}", animatable->getName());
		return nullptr;
	}
	return targetAnimation;
}










bool SequenceAnimation::onSave(tinyxml2::XMLElement* xml){
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

std::shared_ptr<SequenceAnimation> SequenceAnimation::load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable){
	auto sequenceAnimation = std::make_shared<SequenceAnimation>(animatable);
	if(!sequenceAnimation->interpolationType->load(xml)){
		Logger::warn("could not load attribute Interpolation Type of Animation {}", animatable->getName());
	}
	if(!sequenceAnimation->start->load(xml)){
		Logger::warn("could not load attribute Start of Animation {}", animatable->getName());
		return nullptr;
	}
	if(!sequenceAnimation->target->load(xml)){
		Logger::warn("could not load attribute End of Animation {}", animatable->getName());
		return nullptr;
	}
	if(!sequenceAnimation->duration->load(xml)){
		Logger::warn("could not load attribute Duration of Animation {}", animatable->getName());
		return nullptr;
	}
	if(!sequenceAnimation->timeOffset->load(xml)){
		Logger::warn("could not load attribute Time Offset of Animation {}", animatable->getName());
		return nullptr;
	}
	if(!sequenceAnimation->inAcceleration->load(xml)){
		Logger::warn("could not load attribute inAcceleration of Animation {}", animatable->getName());
		return nullptr;
	}
	if(!sequenceAnimation->outAcceleration->load(xml)){
		Logger::warn("could not load attribute outAcceleration of Animation {}", animatable->getName());
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
		points.clear();
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
