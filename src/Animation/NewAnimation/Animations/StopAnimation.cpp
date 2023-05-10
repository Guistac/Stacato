#include "StopAnimation.h"

#include "../Animatable.h"

namespace AnimationSystem{

	void StopAnimation::onConstruction(){
		Animation::onConstruction();
	}

	bool StopAnimation::onSerialization(){
		bool success = Animation::onSerialization();
		success &= stopRampParameter->serializeIntoParent(this);
		return success;
	}

	bool StopAnimation::onDeserialization(){
		bool success = Animation::onDeserialization();
		success &= stopRampParameter->deserializeFromParent(this);
		return true;
	}

	void StopAnimation::onSetAnimatable(){
		auto leafAnimatable = animatable->downcasted_shared_from_this<LeafAnimatable>();
		stopRampParameter = leafAnimatable->createParameter();
		stopRampParameter->setName("Stop Ramp");
		stopRampParameter->setSaveString("StopRamp");
	}

};
