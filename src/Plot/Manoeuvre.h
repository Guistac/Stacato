#pragma once

class ParameterTrack;
class AnimatableParameter;

#include <tinyxml2.h>

class Manoeuvre{
public:

	Manoeuvre() {}
	Manoeuvre(const Manoeuvre& original);

	char name[64] = "";
	char description[256] = "";

	std::vector<std::shared_ptr<ParameterTrack>> tracks;

	void addTrack(std::shared_ptr<AnimatableParameter>& parameter);
	void removeTrack(std::shared_ptr<AnimatableParameter>& parameter);
	bool hasTrack(std::shared_ptr<AnimatableParameter>& parameter);
	double getLength_seconds();

	void listGui();
	void editGui();

	bool save(tinyxml2::XMLElement* manoeuvreXML);
	bool load(tinyxml2::XMLElement* manoeuvreXML);

};

