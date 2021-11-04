#include <pch.h>

#include "Plot.h"

#include <tinyxml2.h>

#include "Plot/Manoeuvre.h"

void Plot::selectManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre) {
	selectedManoeuvre = manoeuvre;
}

std::shared_ptr<Manoeuvre> Plot::getSelectedManoeuvre() {
	return selectedManoeuvre;
}

void Plot::addManoeuvre() {
	std::shared_ptr<Manoeuvre> newManoeuvre = std::make_shared<Manoeuvre>();
	sprintf(newManoeuvre->name, "M-%i", (int)manoeuvres.size());
	if (selectedManoeuvre == nullptr) manoeuvres.push_back(newManoeuvre);
	else {
		for (int i = 0; i < manoeuvres.size(); i++) {
			if (manoeuvres[i] == selectedManoeuvre) {
				manoeuvres.insert(manoeuvres.begin() + i + 1, newManoeuvre);
				break;
			}
		}
	}
}

void Plot::deleteSelectedManoeuvre() {
	if (selectedManoeuvre != nullptr) {
		for (int i = 0; i < manoeuvres.size(); i++) {
			if (manoeuvres[i] == selectedManoeuvre) {
				manoeuvres.erase(manoeuvres.begin() + i);
				break;
			}
		}
	}
	selectedManoeuvre = nullptr;
}

void Plot::duplicateSelectedManoeuvre() {
	if (selectedManoeuvre != nullptr) {
		std::shared_ptr<Manoeuvre> copiedManoeuvre = std::make_shared<Manoeuvre>(*selectedManoeuvre);
		sprintf(copiedManoeuvre->name, "%s*", selectedManoeuvre->name);
		sprintf(copiedManoeuvre->description, "(copy) %s", selectedManoeuvre->description);
		for (int i = 0; i < manoeuvres.size(); i++) {
			if (manoeuvres[i] == selectedManoeuvre) {
				manoeuvres.insert(manoeuvres.begin() + i + 1, copiedManoeuvre);
				break;
			}
		}
	}
}





void Plot::primeSelectedManoeuvre() {
	if (selectedManoeuvre) selectedManoeuvre->prime();
}

bool Plot::isSelectedManoeuvrePrimed() {
	return false;
}

void Plot::startSelectedManoeuvre() {
	if (selectedManoeuvre) selectedManoeuvre->startPlayback();
}

void Plot::stopAllManoeuvres() {
	for (auto& manoeuvre : manoeuvres) {
		manoeuvre->stopPlayback();
	}
}







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
	if(document.LoadFile(path) != XML_SUCCESS) return Logger::warn("Could not Open Plot SaveFile");

	XMLElement* plotXML = document.FirstChildElement("Plot");
	if (plotXML == nullptr) return Logger::warn("Could not find plot attribute");

	const char* nameString;
	if (plotXML->QueryStringAttribute("Name", &nameString) != XML_SUCCESS) return Logger::warn("Could not find plot name attribute");
	strcpy(name, nameString);

	XMLElement* manoeuvreXML = plotXML->FirstChildElement("Manoeuvre");
	while (manoeuvreXML != nullptr) {
		std::shared_ptr<Manoeuvre> manoeuvre = std::make_shared<Manoeuvre>();
		if (!manoeuvre->load(manoeuvreXML)) return Logger::warn("Could not load Manoeuvre");
		manoeuvres.push_back(manoeuvre);
		manoeuvreXML = manoeuvreXML->NextSiblingElement("Manoeuvre");
	}

	return true;
}