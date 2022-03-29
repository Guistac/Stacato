#pragma once

#include "Motion/Curve/Curve.h"

class Machine;
class ParameterTrack;
class AnimatableParameter;
namespace tinyxml2 { class XMLElement; }

enum class ParameterDataType {
	BOOLEAN_PARAMETER,
	INTEGER_PARAMETER,
	STATE_PARAMETER,
	REAL_PARAMETER,
	VECTOR_2D_PARAMETER,
	VECTOR_3D_PARAMETER,
	POSITION,
	POSITION_2D,
	POSITION_3D,
	PARAMETER_GROUP
};

#define ParameterDataTypeStrings \
	{ParameterDataType::BOOLEAN_PARAMETER, 		"Boolean", 			"Boolean"},\
	{ParameterDataType::INTEGER_PARAMETER, 		"Integer", 			"Integer"},\
	{ParameterDataType::STATE_PARAMETER, 		"State", 			"State"},\
	{ParameterDataType::REAL_PARAMETER, 		"Real", 			"Real"},\
	{ParameterDataType::VECTOR_2D_PARAMETER, 	"2D Vector", 		"2DVector"},\
	{ParameterDataType::VECTOR_3D_PARAMETER, 	"3D Vector", 		"3DVector"},\
	{ParameterDataType::POSITION, 				"Position", 		"Position"},\
	{ParameterDataType::POSITION_2D, 			"2D Position", 		"3DPosition"},\
	{ParameterDataType::POSITION_3D, 			"3D Position", 		"3DPosition"},\
	{ParameterDataType::PARAMETER_GROUP, 		"Paramater Group", 	"ParameterGroup"}\

DEFINE_ENUMERATOR(ParameterDataType, ParameterDataTypeStrings)




//=== value structure for State Data Type ===
struct StateParameterValue {
	int integerEquivalent;
	const char displayName[64];
	const char saveName[64];
};



struct AnimatableParameterValue {
public:
	
	//=== Base Information ===
	std::shared_ptr<AnimatableParameter> parameter;

	//=== Value Data ===
	union {
		bool boolean;
		int integer;
		double real;
		glm::vec2 vector2;
		glm::vec3 vector3;
		StateParameterValue* state;
	};

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
	AnimatableParameter(const char* nm);
	AnimatableParameter(const char* nm, ParameterDataType datat);
	AnimatableParameter(const char* nm, ParameterDataType datat, Unit unit);

	//Constructor for Parameter with State DataType
	AnimatableParameter(const char* nm, std::vector<StateParameterValue>* stateValues);

	//Constructor for Parameter Group
	AnimatableParameter(const char* nm, std::vector<std::shared_ptr<AnimatableParameter>> children);
	
	//=== Basic Parameter Information ===
	ParameterDataType dataType;
	char name[128];
	std::shared_ptr<Machine> machine;

	//=== For Non-Group Parameters ===
	std::vector<Motion::InterpolationType> getCompatibleInterpolationTypes();
	Unit unit;

	//=== For Parameters with State Type ===
	std::vector<StateParameterValue>* stateParameterValues = nullptr;
	std::vector<StateParameterValue>& getStateValues() { return *stateParameterValues; }

	//=== For Group Parameters or Parameters in a group ===
	std::vector<std::shared_ptr<AnimatableParameter>> childParameters;
	std::shared_ptr<AnimatableParameter> parentParameter = nullptr;
	bool hasParentGroup() { return parentParameter != nullptr; }
	bool hasChildParameters() { return !childParameters.empty(); }
	std::shared_ptr<AnimatableParameter> getParentGroup() { return parentParameter; }
	
	//=== For Parameters Controlled by ParameterTrack Animation ===
	std::shared_ptr<ParameterTrack> actualParameterTrack = nullptr;
	bool hasParameterTrack() { return actualParameterTrack != nullptr; }
	void getActiveTrackParameterValue(AnimatableParameterValue& output);
};
