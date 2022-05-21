#include <pch.h>

#include "Plot.h"

#include <tinyxml2.h>

#include "Motion/Manoeuvre/Manoeuvre.h"

bool Plot::save(std::string& filePath) {
	using namespace tinyxml2;

	XMLDocument document;
	XMLElement* plotXML = document.NewElement("Plot");
	document.InsertEndChild(plotXML);
	plotXML->SetAttribute("Name", getName());

	for (auto& manoeuvre : manoeuvres) {
		XMLElement* manoeuvreXML = plotXML->InsertNewChildElement("Manoeuvre");
		if (!manoeuvre->save(manoeuvreXML)) return false;
	}

	return document.SaveFile(filePath.c_str()) == XML_SUCCESS;
}

std::shared_ptr<Plot> Plot::load(std::string& filePath) {
	using namespace tinyxml2;
	
	auto plot = std::make_shared<Plot>();

	XMLDocument document;
	if (document.LoadFile(filePath.c_str()) != XML_SUCCESS) {
		Logger::warn("Could not Open Plot SaveFile");
		return nullptr;
	}

	XMLElement* plotXML = document.FirstChildElement("Plot");
	if (plotXML == nullptr) {
		Logger::warn("Could not find plot attribute");
		return nullptr;
	}

	const char* nameString;
	if (plotXML->QueryStringAttribute("Name", &nameString) != XML_SUCCESS) {
		Logger::warn("Could not find plot name attribute");
		return nullptr;
	}
	plot->setName(nameString);

	XMLElement* manoeuvreXML = plotXML->FirstChildElement("Manoeuvre");
	while (manoeuvreXML != nullptr) {
		auto manoeuvre = Manoeuvre::load(manoeuvreXML);
		if(manoeuvre == nullptr) return nullptr;
		plot->manoeuvres.push_back(manoeuvre);
		manoeuvreXML = manoeuvreXML->NextSiblingElement("Manoeuvre");
	}
	plot->refreshAll();

	return plot;
}
