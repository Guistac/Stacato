#include <pch.h>

#include "Manoeuvre.h"
#include "Motion/Machine/ParameterSequence.h"


void Manoeuvre::addParameter(std::shared_ptr<AnimatableParameter>& parameter) {
	std::shared_ptr<ParameterSequence> parameterSequence = std::make_shared<ParameterSequence>(parameter, SequenceType::Type::SIMPLE_TIMED_MOVE);
	parameterSequences.push_back(parameterSequence);
}

void Manoeuvre::removeParameter(std::shared_ptr<AnimatableParameter>& parameter) {
	for (int i = 0; i < parameterSequences.size(); i++) {
		if (parameterSequences[i]->parameter == parameter) {
			parameterSequences.erase(parameterSequences.begin() + i);
			break;
		}
	}
}

bool Manoeuvre::hasParameter(std::shared_ptr<AnimatableParameter>& parameter) {
	for (auto& p : parameterSequences) {
		if (p->parameter == parameter) return true;
	}
	return false;
}


bool Manoeuvre::save(tinyxml2::XMLElement* manoeuvreXML) {
	manoeuvreXML->SetAttribute("Name", name);
	manoeuvreXML->SetAttribute("Description", description);
	return true;
}

bool Manoeuvre::load(tinyxml2::XMLElement* manoeuvreXML) {
	using namespace tinyxml2;
	const char* nameString;
	if (manoeuvreXML->QueryStringAttribute("Name", &nameString) != XML_SUCCESS) return Logger::warn("Could not find Manoeuvre Name Attribute");
	strcpy(name, nameString);
	const char* descriptionString;
	if (manoeuvreXML->QueryStringAttribute("Description", &descriptionString) != XML_SUCCESS) return Logger::warn("Could not find Manoeuvre Description Attribute");
	strcpy(description, descriptionString);
	return true;
}