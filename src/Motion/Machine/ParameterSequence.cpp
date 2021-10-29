#include <pch.h>

#include "ParameterSequence.h"

double ParameterSequence::getLength_seconds() {
	return 0.0;
}


std::vector<SequenceType> sequenceTypes = {
	{SequenceType::Type::SIMPLE_TIMED_MOVE, "Simple Timed Movement", "SimpleTimedMove"},
	{SequenceType::Type::SIMPLE_VELOCITY_MOVE, "Simple Velocity Movement", "SimpleVelocityMove"},
	{SequenceType::Type::COMPLEX_ANIMATED_MOVE, "Complex Animated Movement", "ComplexAnimatedMove"}
};
std::vector<SequenceType>& getSequenceTypes() {
	return sequenceTypes;
}
SequenceType* getSequenceType(SequenceType::Type t) {
	for (auto& sequenceType : sequenceTypes) {
		if (sequenceType.type == t) return &sequenceType;
	}
	return nullptr;
}
SequenceType* getSequenceType(const char* saveName) {
	for (auto& sequenceType : sequenceTypes) {
		if (strcmp(sequenceType.saveName, saveName) == 0) return &sequenceType;
	}
	return nullptr;
}