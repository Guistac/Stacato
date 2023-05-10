#include "TargetAnimation.h"

#include "../Animatable.h"

namespace AnimationSystem{

	void TargetAnimation::onConstruction() {
		Animation::onConstruction();
		
		curveInterpolationTypeParameter = Legato::OptionParameter::createInstance();
		constraintTypeParameter = Legato::OptionParameter::createInstance();
		rampInParameter = Legato::NumberParameter<double>::createInstance(0, "In Ramp", "InRamp");
		rampOutParameter = Legato::NumberParameter<double>::createInstance(0, "Out Ramp", "OutRamp");
	}

	bool TargetAnimation::onSerialization() {
		bool success = Animation::onSerialization();
		success &= curveTargetParameter->serializeIntoParent(this);
		success &= rampInParameter->serializeIntoParent(this);
		success &= rampOutParameter->serializeIntoParent(this);
		return success;
	}

	bool TargetAnimation::onDeserialization() {
		bool success = Animation::onDeserialization();
		success &= curveTargetParameter->deserializeFromParent(this);
		success &= rampInParameter->deserializeFromParent(this);
		success &= rampOutParameter->deserializeFromParent(this);
		return true;
	}

	void TargetAnimation::onSetAnimatable(){
		auto leafAnimatable = animatable->downcasted_shared_from_this<LeafAnimatable>();
		curveTargetParameter = leafAnimatable->createParameter();
		curveTargetParameter->setName("Target");
		curveTargetParameter->setSaveString("Target");
	}

};
