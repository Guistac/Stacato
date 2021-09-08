#include "pch.h"
#include "Environnement.h"

#include "Fieldbus/EtherCatFieldbus.h"

namespace Environnement {

	bool save(const char * filePath) {
		using namespace tinyxml2;

		XMLDocument document;

		XMLElement* environnementXML = document.NewElement("Environnement");
		document.InsertEndChild(environnementXML);
		environnementXML->SetAttribute("name", Environnement::getName());


		//====== NODE GRAPH SAVING ======

		XMLElement* nodeGraphXML = environnementXML->InsertNewChildElement("NodeGraph");
		Environnement::getNodeGraph().save(nodeGraphXML);

		//====== FIELDBUS PARAMETER SAVING ======

		XMLComment* fieldbusSettingsCommentXML = document.NewComment("ETHERCAT FIELDBUS SETTINGS");
		document.InsertEndChild(fieldbusSettingsCommentXML);
		XMLElement* fieldbusSettingsXML = document.NewElement("FieldbusSettings");
		document.InsertEndChild(fieldbusSettingsXML);
		XMLElement* networkInterfaceCardXML = fieldbusSettingsXML->InsertNewChildElement("NetworkInterfaceCard");
		XMLElement* primaryNicXML = networkInterfaceCardXML->InsertNewChildElement("Primary");
		primaryNicXML->SetAttribute("Description", EtherCatFieldbus::networkInterfaceCard.description);
		primaryNicXML->SetAttribute("Name", EtherCatFieldbus::networkInterfaceCard.name);
		if (EtherCatFieldbus::b_redundant) {
			XMLElement* redundantNicXML = networkInterfaceCardXML->InsertNewChildElement("Redundant");
			redundantNicXML->SetAttribute("Description", EtherCatFieldbus::redundantNetworkInterfaceCard.description);
			redundantNicXML->SetAttribute("Name", EtherCatFieldbus::redundantNetworkInterfaceCard.name);
		}
		XMLElement* processIntervalXML = fieldbusSettingsXML->InsertNewChildElement("ProcessInterval");
		processIntervalXML->SetAttribute("Milliseconds", EtherCatFieldbus::processInterval_milliseconds);
		XMLElement* processDataTimeoutXML = fieldbusSettingsXML->InsertNewChildElement("ProcessDataTimeout");
		processDataTimeoutXML->SetAttribute("Milliseconds", EtherCatFieldbus::processDataTimeout_milliseconds);
		XMLElement* clockStableTresholdXML = fieldbusSettingsXML->InsertNewChildElement("ClockStableThreshold");
		clockStableTresholdXML->SetAttribute("Milliseconds", EtherCatFieldbus::clockStableThreshold_milliseconds);
		XMLElement* slaveStateCheckCycleCountXML = fieldbusSettingsXML->InsertNewChildElement("SlaveStateCheckCycleCount");
		slaveStateCheckCycleCountXML->SetAttribute("Cycles", EtherCatFieldbus::slaveStateCheckCycleCount);

		return document.SaveFile(filePath);
	}





	bool load(const char * filePath) {

		using namespace tinyxml2;

		XMLDocument document;
		bool loadResult = document.LoadFile(filePath);

		XMLElement* environnementXML = document.FirstChildElement("Environnement");
		const char* environnementName;
		environnementXML->QueryStringAttribute("name", &environnementName);
		Environnement::setName(environnementName);

		XMLElement* nodeGraphXML = environnementXML->FirstChildElement("NodeGraph");
		Environnement::getNodeGraph().load(nodeGraphXML);


		XMLElement* fieldbusSettingsXML = document.FirstChildElement("FieldbusSettings");
		XMLElement* networkInterfaceCardXML = fieldbusSettingsXML->FirstChildElement("NetworkInterfaceCard");
		XMLElement* primaryNicXML = networkInterfaceCardXML->FirstChildElement("Primary");
		const char* primaryNicDescription;
		const char* primaryNicName;
		primaryNicXML->QueryStringAttribute("Description", &primaryNicDescription);
		primaryNicXML->QueryStringAttribute("Name", &primaryNicName);
		XMLElement* redundantNicXML = networkInterfaceCardXML->FirstChildElement("Redundant");
		const char* redundantNicDescription;
		const char* redundantNicName;
		bool hasRedundantNic = redundantNicXML != nullptr;
		if (hasRedundantNic) {
			redundantNicXML->QueryStringAttribute("Description", &redundantNicDescription);
			redundantNicXML->QueryStringAttribute("Name", &redundantNicName);
		}
		XMLElement* processIntervalXML = fieldbusSettingsXML->FirstChildElement("ProcessInterval");
		float processIntervalMilliseconds;
		processIntervalXML->QueryFloatAttribute("Milliseconds", &processIntervalMilliseconds);
		XMLElement* processDataTimeoutXML = fieldbusSettingsXML->FirstChildElement("ProcessDataTimeout");
		float processDataTimeoutMilliseconds;
		processDataTimeoutXML->QueryFloatAttribute("Milliseconds", &processDataTimeoutMilliseconds);
		XMLElement* clockStableTresholdXML = fieldbusSettingsXML->FirstChildElement("ClockStableThreshold");
		float clockStableThresholdMilliseconds;
		clockStableTresholdXML->QueryFloatAttribute("Milliseconds", &clockStableThresholdMilliseconds);
		XMLElement* slaveStateCheckCycleCountXML = fieldbusSettingsXML->FirstChildElement("SlaveStateCheckCycleCount");
		int slaveStateCheckCycleCount;
		slaveStateCheckCycleCountXML->QueryIntAttribute("Cycles", &slaveStateCheckCycleCount);
		
		/*
		Logger::warn("Fieldbus Parameters:");
		Logger::warn("Primary Nic: {} {}", primaryNicDescription, primaryNicName);
		if(hasRedundantNic) Logger::warn("Redundant Nic: {} {}", redundantNicDescription, redundantNicName);
		Logger::warn("interval: {}  timeout: {}  threshold: {}  stateCheck: {}",
			processIntervalMilliseconds, processDataTimeoutMilliseconds,
			clockStableThresholdMilliseconds, slaveStateCheckCycleCount);
		*/

		return true;
	}

};
