#include <pch.h>

#include "ParameterSequence.h"
#include "Motion/Machine/AnimatableParameter.h"
#include "Motion/Curve/Curve.h"

void ParameterSequence::setInterpolationType(InterpolationType::Type t) {
	curve->interpolationType = t;
	bool sequenceTypeCompatible = false;
	for (auto sequenceT : curve->getCompatibleSequenceTypes()) {
		if (sequenceT == sequenceType) {
			sequenceTypeCompatible = true;
			break;
		}
	}
	if (!sequenceTypeCompatible) {
		setSequenceType(curve->getCompatibleSequenceTypes().front());
	}
}

void ParameterSequence::setSequenceType(SequenceType::Type t) {
	sequenceType = t;
}