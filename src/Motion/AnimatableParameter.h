#pragma once

#include "Curve/Curve.h"

class Machine;
class ParameterTrack;
namespace tinyxml2 {
	class XMLElement;
}

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
		KINEMATIC_3D_POSITION_CURVE,
		PARAMETER_GROUP
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};

std::vector<ParameterDataType>& getParameterDataTypes();
ParameterDataType* getParameterDataType(const char* saveName);
ParameterDataType* getParameterDataType(ParameterDataType::Type t);


//=== value structure for State Data Type ===
struct StateParameterValue {
	int integerEquivalent;
	const char displayName[64];
	const char saveName[64];
};


struct AnimatableParameterValue {
public:

	//=== Base Information ===
	ParameterDataType::Type type;
	const char* shortUnitString = nullptr;

	//=== Value Data ===
	bool boolValue = false;
	int integerValue = false;
	StateParameterValue* stateValue = nullptr;
	double realValue = false;
	glm::vec2 vector2value = glm::vec2(0);
	glm::vec3 vector3value = glm::vec3(0);
	
	//=== State values for parameters with state data type ===
	std::vector<StateParameterValue>* stateValues = nullptr;

	//=== Reset and Comparison ===
	void reset();
	bool equals(AnimatableParameterValue& other);

	//=== Editing Gui ===
	bool inputFieldGui(float width);

	//=== Saving and Loading ===
	bool save(tinyxml2::XMLElement* parameterValueXML);
	bool load(tinyxml2::XMLElement* parameterValueXML);
};


class AnimatableParameter {
public:

	//Constructor for Base Parameter Types
	AnimatableParameter(const char* nm, ParameterDataType::Type datat, const char* unitShortStr);

	//Constructor for Parameter with State DataType
	AnimatableParameter(const char* nm, std::vector<StateParameterValue>* stateValues);

	//Constructor for Parameter Group
	AnimatableParameter(const char* nm, std::vector<std::shared_ptr<AnimatableParameter>> children);

	//=== Basic Parameter Information ===
	ParameterDataType::Type dataType;
	char name[128];
	std::shared_ptr<Machine> machine;

	//=== For Non-Group Parameters ===
	std::vector<InterpolationType::Type> getCompatibleInterpolationTypes();
	char shortUnitString[16];
	
	//=== For Parameters Controlled by ParameterTrack Animation ===
	std::shared_ptr<ParameterTrack> actualParameterTrack = nullptr;
	bool hasParameterTrack() {
		return actualParameterTrack != nullptr;
	}
	void getActiveTrackParameterValue(AnimatableParameterValue& output);

	//=== For Parameters with State Type ===
	std::vector<StateParameterValue>* stateParameterValues = nullptr;
	std::vector<StateParameterValue>& getStateValues() {
		return *stateParameterValues;
	}

	//=== For Group Parameters or Parameters in a group ===
	std::vector<std::shared_ptr<AnimatableParameter>> childParameters;
	std::shared_ptr<AnimatableParameter> parentParameter = nullptr;
	bool hasParentGroup() {
		return parentParameter != nullptr;
	}
	bool hasChildParameters() {
		return !childParameters.empty();
	}
	std::shared_ptr<AnimatableParameter> getParentGroup() {
		return parentParameter;
	}
};