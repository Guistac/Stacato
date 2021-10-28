#pragma once

class Machine;

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

	std::shared_ptr<Machine> machine;

	//data:
	bool boolData;
	int intData;
	double realData;
	glm::vec2 vec2Data;
	glm::vec3 vec3Data;
};
