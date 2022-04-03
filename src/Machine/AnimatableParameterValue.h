#pragma once

class AnimatableParameter;
struct StateParameterValue;

namespace tinyxml2{ class XMLElement; }

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
