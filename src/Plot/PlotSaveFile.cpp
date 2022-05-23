#include <pch.h>

#include "Plot.h"

#include <tinyxml2.h>

#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Plot/ManoeuvreList.h"






bool Plot::save(std::string& filePath) {
	using namespace tinyxml2;

	XMLDocument document;
	XMLElement* plotXML = document.NewElement("Plot");
	document.InsertEndChild(plotXML);
	plotXML->SetAttribute("Name", getName());

	XMLElement* manoeuvreListXML = plotXML->InsertNewChildElement("ManoeuvreList");
	manoeuvreList->save(manoeuvreListXML);

	return document.SaveFile(filePath.c_str()) == XML_SUCCESS;
	
	return false;
}

std::shared_ptr<Plot> Plot::load(std::string& filePath) {
	using namespace tinyxml2;
	
	auto plot = Plot::create();

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
	
	XMLElement* manoeuvreListXML = plotXML->FirstChildElement("ManoeuvreList");
	if(manoeuvreListXML == nullptr){
		Logger::warn("Could not find ManoeuvreList attribute");
		return nullptr;
	}
	plot->manoeuvreList->load(manoeuvreListXML);
	 
	return plot;
}












bool ManoeuvreList::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* manoeuvreXML = xml->FirstChildElement("Manoeuvre");
	while (manoeuvreXML != nullptr) {
		auto manoeuvre = Manoeuvre::load(manoeuvreXML);
		if(manoeuvre == nullptr) return false;
		manoeuvre->setManoeuvreList(shared_from_this());
		manoeuvres.push_back(manoeuvre);
		manoeuvreXML = manoeuvreXML->NextSiblingElement("Manoeuvre");
	}
	//manoeuvreList->refreshAll();
	return true;
}

bool ManoeuvreList::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	for (auto& manoeuvre : manoeuvres) {
		XMLElement* manoeuvreXML = xml->InsertNewChildElement("Manoeuvre");
		if (!manoeuvre->save(manoeuvreXML)) return false;
	}
	return true;
}
