#include "pch.h"
#include "Environnement.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "NodeGraph.h"
#include "Gui/Environnement/NodeGraph/NodeGraphGui.h"

#include <tinyxml2.h>

namespace Environnement {

	bool save(const char * filePath) {
		using namespace tinyxml2;

		XMLDocument document;

		//====== ENVIRONNEMENT SAVING ======

		XMLElement* environnementXML = document.NewElement("Environnement");
		document.InsertEndChild(environnementXML);
		environnementXML->SetAttribute("name", Environnement::getName());
		environnementXML->SetAttribute("notes", Environnement::getNotes());

		//====== NODE GRAPH SAVING ======

		XMLElement* nodeGraphXML = environnementXML->InsertNewChildElement("NodeGraph");
		Environnement::NodeGraph::save(nodeGraphXML);

		//====== FIELDBUS PARAMETER SAVING ======

		XMLElement* fieldbusSettingsXML = document.NewElement("FieldbusSettings");
		document.InsertEndChild(fieldbusSettingsXML);
		EtherCatFieldbus::save(fieldbusSettingsXML);

		return XML_SUCCESS == document.SaveFile(filePath);
	}





	bool load(const char * filePath) {
		using namespace tinyxml2;

		//====== DOCUMENT LOADING ======

		Logger::info("Loading Save File {}", filePath);

		XMLDocument document;
		XMLError loadResult = document.LoadFile(filePath);
		if (loadResult != XML_SUCCESS) return Logger::warn("Could not Open SaveFile (tinyxml2 error: {})", XMLDocument::ErrorIDToName(loadResult));
		
		//====== ENVIRONNEMENT LOADING ======

		XMLElement* environnementXML = document.FirstChildElement("Environnement");
		if (!environnementXML) return Logger::warn("Could not load Environnement from SaveFile");
		const char* environnementName;
		if (environnementXML->QueryStringAttribute("name", &environnementName) != XML_SUCCESS) return Logger::warn("Could not load Environnement name");
		Environnement::setName(environnementName);
		const char* environnementNotes;
		if(environnementXML->QueryStringAttribute("notes", &environnementNotes) != XML_SUCCESS) return Logger::warn("Could not load Environnement notes");
		Environnement::setNotes(environnementNotes);

		//====== NODE GRAPH LOADING ======

		XMLElement* nodeGraphXML = environnementXML->FirstChildElement("NodeGraph");
		if (!nodeGraphXML) return Logger::warn("Could not load NodeGraph from SaveFile");
		if (!Environnement::NodeGraph::load(nodeGraphXML)) return Logger::warn("Error reading NodeGraph data");

		//====== FIELDBUS PARAMETER LOADING ======
		
		XMLElement* fieldbusSettingsXML = document.FirstChildElement("FieldbusSettings");
		if (!fieldbusSettingsXML) return Logger::warn("Could not load Fieldbus Settings from SaveFile");
		if (!EtherCatFieldbus::load(fieldbusSettingsXML)) return Logger::warn("Error reading Fieldbus settings data");

		return Logger::info("Successfully loaded Save File");
	}

};
