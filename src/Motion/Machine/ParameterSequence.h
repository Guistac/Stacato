#pragma once

class AnimatableParameter;
class CurvePoint;


struct SequenceType {
	enum class Type {
		SIMPLE_TIMED_MOVE,
		SIMPLE_VELOCITY_MOVE,
		COMPLEX_ANIMATED_MOVE
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};

std::vector<SequenceType>& getSequenceTypes();
SequenceType* getSequenceType(SequenceType::Type t);
SequenceType* getSequenceType(const char* saveName);



class ParameterSequence{
public:

	ParameterSequence(std::shared_ptr<AnimatableParameter>& param, SequenceType::Type t) : parameter(param), type(t) {}

	std::shared_ptr<AnimatableParameter> parameter;
	SequenceType::Type type;
	
	/*
	machine
	parameter
	sequencetype
	target
	constraint(vel/time)
	offsettime
	rampIn
	rampEqual
	rampOut
	*/

	std::vector<std::shared_ptr<CurvePoint>> points;

	double getLength_seconds();

	

};
