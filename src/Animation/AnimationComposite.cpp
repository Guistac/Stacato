/*
#include "Animation.h"


std::shared_ptr<AnimationComposite> AnimationComposite::copy(){
	auto compositeCopy = std::make_shared<AnimationComposite>(getAnimatable()->toComposite());
	for(auto childAnimation : getChildren()){
		auto childAnimationCopy = childAnimation->copy();
		childAnimationCopy->setParentComposite(compositeCopy);
		compositeCopy->children.push_back(childAnimationCopy);
	}
	return compositeCopy;
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
		std::shared_ptr<Animation> childAnimation = Animation::load(xml, animatable);
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

*/
