#include <pch.h>

#include "NodeGraph/NodeGraph.h"

#include <tinyxml2.h>

bool NodeGraph::save(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* nodes = xml->InsertNewChildElement("Nodes");

	for (auto node : getIoNodes()) {
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
	XMLElement* links = xml->InsertNewChildElement("Links");
	for (auto link : getIoLinks()) {
		XMLElement* linkXML = links->InsertNewChildElement("Link");
		linkXML->SetAttribute("UniqueID", link->getUniqueID());
		linkXML->SetAttribute("StartPin", link->getInputData()->getUniqueID());
		linkXML->SetAttribute("EndPin", link->getOutputData()->getUniqueID());
	}

	return true;
}

bool NodeGraph::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* nodesXML = xml->FirstChildElement("Nodes");
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

	XMLElement* linksXML = xml->FirstChildElement("Links");
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

	return true;
}
