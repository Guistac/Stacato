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
		XMLElement* nodes = nodeGraphXML->InsertNewChildElement("Nodes");
		for (auto node : Environnement::getNodeGraph().getIoNodes()) {
			XMLElement* nodeXML = nodes->InsertNewChildElement("Node");

			nodeXML->SetAttribute("ClassName", node->getNodeName());
			nodeXML->SetAttribute("UniqueID", node->getUniqueID());
			nodeXML->SetAttribute("NodeType", node->getTypeString());
			if (node->getType() == NodeType::IODEVICE) {
				nodeXML->SetAttribute("DeviceType", node->getDeviceTypeString());
			}
			nodeXML->SetAttribute("customName", node->getName());

			XMLElement* positionXML = nodeXML->InsertNewChildElement("NodeEditorPosition");
			float xPosition, yPosition;
			node->getNodeGraphPosition(xPosition, yPosition);
			positionXML->SetAttribute("x", xPosition);
			positionXML->SetAttribute("y", yPosition);

			XMLElement* inputPinsXML = nodeXML->InsertNewChildElement("InputPins");
			for (auto pin : node->getNodeInputData()) {
				XMLElement* inputPinXML = inputPinsXML->InsertNewChildElement("InputPin");
				inputPinXML->SetAttribute("name", pin->getName());
				inputPinXML->SetAttribute("DataType", pin->getTypeName());
				inputPinXML->SetAttribute("UniqueID", pin->getUniqueID());
			}
			XMLElement* outputPinsXML = nodeXML->InsertNewChildElement("OutputPins");
			for (auto pin : node->getNodeOutputData()) {
				XMLElement* outputPinXML = outputPinsXML->InsertNewChildElement("OutputPin");
				outputPinXML->SetAttribute("name", pin->getName());
				outputPinXML->SetAttribute("DataType", pin->getTypeName());
				outputPinXML->SetAttribute("UniqueID", pin->getUniqueID());
			}

		}
		XMLElement* links = nodeGraphXML->InsertNewChildElement("Links");
		for (auto link : Environnement::getNodeGraph().getIoLinks()) {
			XMLElement* linkXML = links->InsertNewChildElement("Link");
			linkXML->SetAttribute("UniqueID", link->getUniqueID());
			linkXML->SetAttribute("StartPin", link->getInputData()->getUniqueID());
			linkXML->SetAttribute("EndPin", link->getOutputData()->getUniqueID());
		}

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
		XMLElement* nodesXML = nodeGraphXML->FirstChildElement("Nodes");
		XMLElement* nodeXML = nodesXML->FirstChildElement("Node");
		while (nodeXML) {
			const char* className;
			nodeXML->QueryStringAttribute("ClassName", &className);
			int nodeUniqueID;
			nodeXML->QueryIntAttribute("UniqueID", &nodeUniqueID);
			const char* nodeType;
			nodeXML->QueryStringAttribute("NodeType", &nodeType);
			const char* deviceType;
			bool isDeviceNode = nodeXML->QueryStringAttribute("DeviceType", &deviceType) == XML_SUCCESS;

			XMLElement* positionXML = nodeXML->FirstChildElement("NodeEditorPosition");
			float xPosition, yPosition;
			positionXML->QueryFloatAttribute("x", &xPosition);
			positionXML->QueryFloatAttribute("y", &yPosition);

			//Logger::warn("Node class: {}  ID: {}  Type: {}", className, nodeUniqueID, nodeType);
			if (isDeviceNode) Logger::warn("DeviceType: {}", deviceType);
			//Logger::warn("NodePos x{} y{}", xPosition, yPosition);

			XMLElement* inputPinsXML = nodeXML->FirstChildElement("InputPins");
			XMLElement* inputPinXML = inputPinsXML->FirstChildElement("InputPin");
			while (inputPinXML) {
				const char* pinName;
				inputPinXML->QueryStringAttribute("name", &pinName);
				const char* pinDataType;
				inputPinXML->QueryStringAttribute("DataType", &pinDataType);
				int pinUniqueID;
				inputPinXML->QueryIntAttribute("UniqueID", &pinUniqueID);
				//Logger::warn("InputPin name: {}  dataType: {}  ID: {}", pinName, pinDataType, pinUniqueID);
				inputPinXML = inputPinXML->NextSiblingElement();
			}
			XMLElement* outputPinsXML = nodeXML->FirstChildElement("OutputPins");
			XMLElement* outputPinXML = outputPinsXML->FirstChildElement("OutputPin");
			while (outputPinXML) {
				const char* pinName;
				outputPinXML->QueryStringAttribute("name", &pinName);
				const char* pinDataType;
				outputPinXML->QueryStringAttribute("DataType", &pinDataType);
				int pinUniqueID;
				outputPinXML->QueryIntAttribute("UniqueID", &pinUniqueID);
				//Logger::warn("OutputPin name: {}  dataType: {}  ID: {}", pinName, pinDataType, pinUniqueID);
				outputPinXML = outputPinXML->NextSiblingElement();
			}

			nodeXML = nodeXML->NextSiblingElement();
		}

		XMLElement* linksXML = nodeGraphXML->FirstChildElement("Links");
		XMLElement* linkXML = linksXML->FirstChildElement("Link");
		while (linkXML) {
			int linkUniqueID;
			int startPin, endPin;
			linkXML->QueryIntAttribute("UniqueID", &linkUniqueID);
			linkXML->QueryIntAttribute("StartPin", &startPin);
			linkXML->QueryIntAttribute("EndPin", &endPin);
			//Logger::warn("Link ID: {}  Start: {}  End: {}", linkUniqueID, startPin, endPin);
			linkXML = linkXML->NextSiblingElement("Link");
		}


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
