#pragma once

#include "Curve/Curve.h"

class Machine;
class ParameterTrack;

#include <tinyxml2.h>

struct ParameterDataType {
	enum class Type {
		BOOLEAN_PARAMETER,
		INTEGER_PARAMETER,
		STATE_PARAMETER,
		REAL_PARAMETER,
		VECTOR_2D_PARAMETER,
		VECTOR_3D_PARAMETER,
		KINEMATIC_POSITION_CURVE,
		KINEMATIC_2D_POSITION_CURVE,
		KINEMATIC_3D_POSITION_CURVE
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};

std::vector<ParameterDataType>& getParameterDataTypes();
ParameterDataType* getParameterDataType(const char* saveName);
ParameterDataType* getParameterDataType(ParameterDataType::Type t);


struct StateParameterValue {
	int integerEquivalent;
	const char displayName[64];
	const char saveName[64];
};

struct AnimatableParameterValue {
public:

	ParameterDataType::Type type;

	bool inputFieldGui(float width);

	std::vector<StateParameterValue>* stateValues = nullptr;

	bool boolValue = false;
	int integerValue = false;
	StateParameterValue* stateValue = nullptr;
	double realValue = false;
	glm::vec2 vector2value = glm::vec2(0);
	glm::vec3 vector3value = glm::vec3(0);

	bool save(tinyxml2::XMLElement* parameterValueXML);
	bool load(tinyxml2::XMLElement* parameterValueXML);

};


class AnimatableParameter {
public:

	AnimatableParameter(const char* nm, std::shared_ptr<Machine> mach, ParameterDataType::Type datat) : machine(mach), dataType(datat) {
		strcpy(name, nm);
	}

	AnimatableParameter(const char* nm, std::shared_ptr<Machine> mach, std::vector<StateParameterValue>* stateValues) : machine(mach), stateParameterValues(stateValues) {
		strcpy(name, nm);
		dataType = ParameterDataType::Type::STATE_PARAMETER;
	}


	ParameterDataType::Type dataType;
	std::vector<InterpolationType::Type> getCompatibleInterpolationTypes();

	char name[128];
	std::shared_ptr<Machine> machine;

	//parameter limit query

	//TYPE: boolean
	//no limits

	//TYPE: state / integer
	//inside 1D range / get 1D range

	//TYPE: 1D real
	//inside 1D range / get 1D range (lower upper)
	//1D velocity limit / 1D acceleration limit
	//inside Error treshold / get Error threshold

	//TYPE: 2D real
	//inside 2D range / get 2D range (2D mesh ?)
	//2D velocity limit / 2D acceleration limit
	//inside Error threshold / get Error treshold

	//TYPE: 3D real
	//inside 3D range / get 3D range (3D mesh ?)
	//3D velocity limit / 3D acceleration limit
	//inside Error Threshold / get Error Treshold

	bool isUnderVelocityLimit(double vel);
	//bool isUnderVelocityLimit(glm::vec2 vel2);
	//bool isUnderVelocityLimit(glm::vec3 vel3);

	double getVelocityLimit1D();
	//glm::vec2 getVelocityLimit2D();
	//glm::vec3 getVelocityLimit3D();

	bool isUnderAccelerationLimit(double acc);
	//bool isUnderAccelerationLimit(glm::vec2 acc2);
	//bool isUnderAccelerationLimit(glm::vec3 acc3);

	double getAccelerationLimit1D();
	//glm::vec2 getAccelerationLimit2D();
	//glm::vec3 getAccelerationLimit3D();

	bool isInsideRange(int pos);
	bool isInsideRange(double pos);
	///bool isInsideRange(glm::vec2 pos2);
	//bool isInsideRange(glm::vec3 pos3);

	int getRangeInt();
	double getRange1D();
	//double getRange2D();
	//double getRange3D();

	bool isInsideErrorTreshold(double pos);
	//bool isInsideErrorTreshold(glm::vec2 pos2);
	//bool isInsideErrorTreshold(glm::vec3 pos3);

	
	void getActiveTrackParameterValue(AnimatableParameterValue& output);
	AnimatableParameterValue& getActualMachineParameterValue() { return actualValue; }
	
	bool hasParameterTrack() { return actualParameterTrack != nullptr; }
	std::shared_ptr<ParameterTrack> actualParameterTrack = nullptr;

	AnimatableParameterValue actualValue;
	AnimatableParameterValue requestedValue;

	//this is limit interrogation in action
	std::vector<StateParameterValue>* stateParameterValues;

	//TODO: parameter units
};