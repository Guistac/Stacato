#pragma once

class ParameterSequence;
class AnimatableParameter;

#include <tinyxml2.h>

class Manoeuvre{
public:

	char name[64] = "";
	char description[256] = "";

	std::vector<std::shared_ptr<ParameterSequence>> parameterSequences;

	void addParameter(std::shared_ptr<AnimatableParameter>& parameter);
	void removeParameter(std::shared_ptr<AnimatableParameter>& parameter);
	bool hasParameter(std::shared_ptr<AnimatableParameter>& parameter);

	void listGui();
	void editGui();

	bool save(tinyxml2::XMLElement* manoeuvreXML);
	bool load(tinyxml2::XMLElement* manoeuvreXML);

};

