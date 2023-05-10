#include "SequenceAnimation.h"

#include "../Animatable.h"

namespace AnimationSystem{


	void SequenceAnimation::onConstruction(){
		Animation::onConstruction();
		curveDurationParameter = Legato::TimeParameter::createInstance(0, "Duration", "Duration");
		rampInParameter = Legato::NumberParameter<double>::createInstance(0, "In Ramp", "InRamp");
		rampOutParameter = Legato::NumberParameter<double>::createInstance(0, "Out Ramp", "OutRamp");
	}

	bool SequenceAnimation::onSerialization(){
		bool success = Animation::onSerialization();
		success &= curveStartParameter->serializeIntoParent(this);
		success &= curveTargetParameter->serializeIntoParent(this);
		success &= curveDeltaParameter->serializeIntoParent(this);
		success &= curveDurationParameter->serializeIntoParent(this);
		success &= rampInParameter->serializeIntoParent(this);
		success &= rampOutParameter->serializeIntoParent(this);
		return success;
	}

	bool SequenceAnimation::onDeserialization(){
		bool success = Animation::onDeserialization();
		success &= curveStartParameter->deserializeFromParent(this);
		success &= curveTargetParameter->deserializeFromParent(this);
		success &= curveDeltaParameter->deserializeFromParent(this);
		success &= curveDurationParameter->deserializeFromParent(this);
		success &= rampInParameter->deserializeFromParent(this);
		success &= rampOutParameter->deserializeFromParent(this);
		return true;
	}

	void SequenceAnimation::onSetAnimatable(){
		auto leafAnimatable = animatable->downcasted_shared_from_this<LeafAnimatable>();
		curveStartParameter = leafAnimatable->createParameter();
		curveStartParameter->setName("Start");
		curveStartParameter->setSaveString("Start");
		curveTargetParameter = leafAnimatable->createParameter();
		curveTargetParameter->setName("Target");
		curveTargetParameter->setSaveString("Target");
		curveDeltaParameter = leafAnimatable->createParameter();
		curveDeltaParameter->setName("Delat");
		curveDeltaParameter->setSaveString("Delta");
	}

};
