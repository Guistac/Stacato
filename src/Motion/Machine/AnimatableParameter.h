#pragma once

class Machine;
class CurvePoint;

class AnimatableParameter {
public:

	enum class Type {
		BOOLEAN_PARAMETER,
		INTEGER_PARAMETER,
		REAL_PARAMETER,
		VECTOR_2D_PARAMETER,
		VECTOR_3D_PARAMETER,
		KINEMATIC_POSITION_CURVE,
		KINEMATIC_2D_POSITION_CURVE,
		KINEMATIC_3D_POSITION_CURVE
	};

	AnimatableParameter(const char* nm, std::shared_ptr<Machine> mach, Type datat) : machine(mach), dataType(datat) {
		strcpy(name, nm);
	}

	char name[128];
	Type dataType;
	std::shared_ptr<Machine> machine;




	//TODO: constraint specific data (limits, kinematic contraints, ...)
};
