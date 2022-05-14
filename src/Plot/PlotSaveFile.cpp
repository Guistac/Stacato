#include <pch.h>

#include "Plot.h"

#include <tinyxml2.h>

#include "Motion/Manoeuvre/Manoeuvre.h"

bool Plot::save(const char* path) {
	using namespace tinyxml2;

	XMLDocument document;
	XMLElement* plotXML = document.NewElement("Plot");
	document.InsertEndChild(plotXML);
	plotXML->SetAttribute("Name", name);

	for (auto& manoeuvre : manoeuvres) {
		XMLElement* manoeuvreXML = plotXML->InsertNewChildElement("Manoeuvre");
		if (!manoeuvre->save(manoeuvreXML)) return false;
	}

	return document.SaveFile(path) == XML_SUCCESS;
}

bool Plot::load(const char* path) {
	using namespace tinyxml2;

	XMLDocument document;
	if (document.LoadFile(path) != XML_SUCCESS) return Logger::warn("Could not Open Plot SaveFile");

	XMLElement* plotXML = document.FirstChildElement("Plot");
	if (plotXML == nullptr) return Logger::warn("Could not find plot attribute");

	const char* nameString;
	if (plotXML->QueryStringAttribute("Name", &nameString) != XML_SUCCESS) return Logger::warn("Could not find plot name attribute");
	strcpy(name, nameString);

	std::shared_ptr<Plot> thisPlot = shared_from_this();

	XMLElement* manoeuvreXML = plotXML->FirstChildElement("Manoeuvre");
	while (manoeuvreXML != nullptr) {
		/*
		std::shared_ptr<Manoeuvre> manoeuvre = std::make_shared<Manoeuvre>(thisPlot);
		if (!manoeuvre->load(manoeuvreXML)) return Logger::warn("Could not load Manoeuvre");
		manoeuvres.push_back(manoeuvre);
		manoeuvreXML = manoeuvreXML->NextSiblingElement("Manoeuvre");
		 */
		break;
	}
	refreshAll();

	return true;
}
