#pragma once

class AnimatableParameter;

#include <tinyxml2.h>

class Manoeuvre{
public:

	char name[64] = "";
	char description[256] = "";

	std::vector<std::shared_ptr<AnimatableParameter>> parameters;

	void listGui();
	void editGui();

	bool save(tinyxml2::XMLElement* manoeuvreXML);
	bool load(tinyxml2::XMLElement* manoeuvreXML);

};

