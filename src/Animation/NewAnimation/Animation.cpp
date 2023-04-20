#include <pch.h>

#include "Animation.h"

namespace AnimationSystem{


std::shared_ptr<Animation> Animation::createInstanceFromTypeString(bool b_isComposite, std::string typeString){
	if(b_isComposite){
		auto compositeAnimation = CompositeAnimation::createInstance();
		if(typeString == "Target") compositeAnimation->setType(AnimationType::TARGET);
		else if(typeString == "Sequence") compositeAnimation->setType(AnimationType::SEQUENCE);
		else if(typeString == "Stop") compositeAnimation->setType(AnimationType::STOP);
		else return nullptr;
		return compositeAnimation;
	}
	else{
		if(typeString == "Target") return TargetAnimation::createInstance();
		else if(typeString == "Sequence") return SequenceAnimation::createInstance();
		else if(typeString == "Stop") return StopAnimation::createInstance();
		else return nullptr;
	}
}


};
