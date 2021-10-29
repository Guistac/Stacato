#pragma once

class Machine;
class CurvePoint;

class AnimatableParameter {
public:

	enum class Type {
		BOOLEAN_PARAMETER,
		INTEGER_PARAMETER,
		REAL_PARAMETER,
		VECTOR_2D,
		VECTOR_3D
	};

	enum class Constraint {
		NONE,
		CLAMP, //constrain only the value itself
		POSITION_PROFILE //constrain position, velocity and acceleration
	};

	AnimatableParameter(const char* nm, std::shared_ptr<Machine> mach, Type datat, Constraint constr) : machine(mach), dataType(datat), constraint(constr) {
		strcpy(name, nm);
	}

	char name[128];
	Type dataType;
	Constraint constraint;
	//TODO: constraint specific data (limits, kinematic contraints, ...)

	std::shared_ptr<Machine> machine;

};

struct KeyFrame {
	//data:
	bool boolData;
	int intData;
	double realData;
	//CurvePoint positionCurvePointData;
};
