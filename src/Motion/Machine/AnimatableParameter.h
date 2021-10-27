#pragma once

class AnimatableParameter {
public:

	enum class Type {
		BOOLEAN_PARAMETER,
		INTEGER_PARAMETER,
		REAL_PARAMETER
	};

	enum class Constraint {
		NONE,
		POSITION_PROFILE //constrain profile ends to zero velocity
	};

	char name[128];
	Type dataType;
	Constraint constraint;

	//data:
	bool boolData;
	int intData;
	double realData;

};
