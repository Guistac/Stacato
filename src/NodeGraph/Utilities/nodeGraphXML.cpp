#include <pch.h>

#include "NodeGraph/NodeGraph.h"

#include "ioNodeFactory.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"

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
		nodeXML->SetAttribute("CustomName", node->getName());

		XMLElement* positionXML = nodeXML->InsertNewChildElement("NodeEditorPosition");
		float xPosition, yPosition;
		node->getNodeGraphPosition(xPosition, yPosition);
		positionXML->SetAttribute("x", xPosition);
		positionXML->SetAttribute("y", yPosition);

		XMLElement* nodeSpecificDataXML = nodeXML->InsertNewChildElement("NodeSpecificData");
		node->save(nodeSpecificDataXML);

		XMLElement* inputPinsXML = nodeXML->InsertNewChildElement("InputPins");
		for (auto pin : node->getNodeInputData()) {
			XMLElement* inputPinXML = inputPinsXML->InsertNewChildElement("InputPin");
			pin->save(inputPinXML);
		}
		XMLElement* outputPinsXML = nodeXML->InsertNewChildElement("OutputPins");
		for (auto pin : node->getNodeOutputData()) {
			XMLElement* outputPinXML = outputPinsXML->InsertNewChildElement("OutputPin");
			pin->save(outputPinXML);
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

	Logger::debug("Loading Nodegraph Data");

	XMLElement* nodesXML = xml->FirstChildElement("Nodes");
	if (!nodesXML) return Logger::warn("Could not load Nodes element from savefile");

	std::vector<std::shared_ptr<ioNode>> loadedNodes;
	std::vector<std::shared_ptr<ioData>> loadedPins;
	std::vector<std::shared_ptr<ioLink>> loadedLinks;
	int largestUniqueID = 0; //TODO: need to increment unique id counter to found biggest value so we can add more elements after loading

	XMLElement* nodeXML = nodesXML->FirstChildElement("Node");
	while (nodeXML) {
		const char* className;
		if (nodeXML->QueryStringAttribute("ClassName", &className) != XML_SUCCESS) return Logger::warn("Could not load Node ClassName");
		int nodeUniqueID;
		if (nodeXML->QueryIntAttribute("UniqueID", &nodeUniqueID) != XML_SUCCESS) return Logger::warn("Could not load Node ID");
		if (nodeUniqueID > largestUniqueID) largestUniqueID = nodeUniqueID;
		const char* nodeCustomName;
		if (nodeXML->QueryStringAttribute("CustomName", &nodeCustomName) != XML_SUCCESS) return Logger::warn("Could not load Node Custom Name");
		const char* nodeType;
		if (nodeXML->QueryStringAttribute("NodeType", &nodeType) != XML_SUCCESS) return Logger::warn("Could not load Node Type");

		Logger::trace("Loading node '{}'", className);
		std::shared_ptr<ioNode> loadedNode = nullptr;
		if (strcmp(nodeType, "PROCESSOR") == 0) loadedNode = ioNodeFactory::getIoNodeByName(className);
		else if (strcmp(nodeType, "IODEVICE") == 0) {
			const char* deviceType;
			if (nodeXML->QueryStringAttribute("DeviceType", &deviceType) != XML_SUCCESS) return Logger::warn("Could not load Node Device Type");
			if (strcmp(deviceType, "ETHERCATSLAVE") == 0) loadedNode = EtherCatDeviceFactory::getDeviceByName(className);
			else if (strcmp(deviceType, "CLOCK") == 0) loadedNode = ioNodeFactory::getIoNodeByName(className);
		}
		else if (strcmp(nodeType, "AXIS")) {}
		else if (strcmp(nodeType, "CONTAINER")) loadedNode = ioNodeFactory::getIoNodeByName(className);
		if (loadedNode == nullptr) return Logger::warn("Coult not load Node Class");
	
		loadedNode->setName(nodeCustomName);
		loadedNode->assignIoData(); //this assigns a unique id if the parent nodegraph is assigned, we don't want that
		loadedNode->uniqueID = nodeUniqueID;
		loadedNode->parentNodeGraph = this;
		loadedNodes.push_back(loadedNode);
		loadedNode->b_isInNodeGraph = true;
		for (std::shared_ptr<ioData> data : loadedNode->nodeInputData) {
			data->parentNode = loadedNode;
			loadedPins.push_back(data);
		}
		for (std::shared_ptr<ioData> data : loadedNode->nodeOutputData) {
			data->parentNode = loadedNode;
			loadedPins.push_back(data);
		}

		XMLElement* positionXML = nodeXML->FirstChildElement("NodeEditorPosition");
		if (!positionXML) return Logger::warn("Could not load node position attribute");
		float xPosition, yPosition;
		positionXML->QueryFloatAttribute("x", &xPosition);
		positionXML->QueryFloatAttribute("y", &yPosition);
		//TODO: set node position

		XMLElement* nodeSpecificDataXML = nodeXML->FirstChildElement("NodeSpecificData");
		if (!nodeSpecificDataXML) return Logger::warn("Could not load Node Specific Data");
		loadedNode->load(nodeSpecificDataXML);

		XMLElement* inputPinsXML = nodeXML->FirstChildElement("InputPins");
		if (!inputPinsXML) return Logger::warn("Could Not Load Node InputPins");
		XMLElement* inputPinXML = inputPinsXML->FirstChildElement("InputPin");
		while (inputPinXML) {
			const char* pinName;
			if (inputPinXML->QueryStringAttribute("Name", &pinName) != XML_SUCCESS) return Logger::warn("Could not load Node Pin Name");
			const char* pinDataType;
			if (inputPinXML->QueryStringAttribute("DataType", &pinDataType) != XML_SUCCESS) return Logger::warn("Could not load Node Pin DataType");
			std::shared_ptr<ioData> matchingPin = nullptr;
			for (auto pin : loadedNode->getNodeInputData()) {
				if (pin->matches(pinName, pinDataType)) {
					if (!pin->load(inputPinXML)) return Logger::warn("Could not load Node Pin '{}'", pinName);
					matchingPin = pin;
					break;
				}
			}
			if (matchingPin == nullptr) return Logger::warn("Could not find pin Matching name: {}  datatype: {}", pinName, pinDataType);
			if (matchingPin->getUniqueID() > largestUniqueID) largestUniqueID = matchingPin->getUniqueID();
			Logger::trace("Loaded Input Pin '{}' with dataType: {} visibility: {}", pinName, pinDataType, matchingPin->isVisible());
			inputPinXML = inputPinXML->NextSiblingElement();
		}
		XMLElement* outputPinsXML = nodeXML->FirstChildElement("OutputPins");
		if (!outputPinsXML) return Logger::warn("Could Not Load Node OutputPins");
		XMLElement* outputPinXML = outputPinsXML->FirstChildElement("OutputPin");
		while (outputPinXML) {
			const char* pinName;
			if (outputPinXML->QueryStringAttribute("Name", &pinName) != XML_SUCCESS) return Logger::warn("Could not load Node Pin Name");
			const char* pinDataType;
			if (outputPinXML->QueryStringAttribute("DataType", &pinDataType) != XML_SUCCESS) return Logger::warn("Could not load Node Pin DataType");
			std::shared_ptr<ioData> matchingPin = nullptr;
			for (auto pin : loadedNode->getNodeOutputData()) {
				if (pin->matches(pinName, pinDataType)) {
					if (!pin->load(outputPinXML)) return Logger::warn("Could not load Node Pin '{}'", pinName);
					matchingPin = pin;
					break;
				}
			}
			if (matchingPin == nullptr) return Logger::warn("Could not find pin Matching name: {}  datatype: {}", pinName, pinDataType);
			if (matchingPin->getUniqueID() > largestUniqueID) largestUniqueID = matchingPin->getUniqueID();
			Logger::trace("Loaded Output Pin '{}' with dataType: {} visibility: {}", pinName, pinDataType, matchingPin->isVisible());
			outputPinXML = outputPinXML->NextSiblingElement();
		}

		Logger::debug("Loaded Node {} (name: '{}') type: {}  ID: {}", className, nodeCustomName, nodeType, nodeUniqueID);
		nodeXML = nodeXML->NextSiblingElement();
	}

	XMLElement* linksXML = xml->FirstChildElement("Links");
	if (!linksXML) return Logger::warn("Could not load node links");
	XMLElement* linkXML = linksXML->FirstChildElement("Link");
	while (linkXML) {
		int linkUniqueID;
		int startPinID, endPinID;
		if (linkXML->QueryIntAttribute("UniqueID", &linkUniqueID) != XML_SUCCESS) return Logger::warn("Could not load link ID");
		if (linkXML->QueryIntAttribute("StartPin", &startPinID) != XML_SUCCESS) return Logger::warn("Could not load link start pin ID");
		if (linkXML->QueryIntAttribute("EndPin", &endPinID) != XML_SUCCESS) return Logger::warn("Could not load link end pin ID");
		
		if (linkUniqueID > largestUniqueID) largestUniqueID = linkUniqueID;

		std::shared_ptr<ioData> startPin;
		for (auto pin : loadedPins) {
			if (pin->getUniqueID() == startPinID) {
				startPin = pin;
				break;
			}
		}
		if (!startPin) return Logger::warn("Could not find matching node pin ID for start pin");
		std::shared_ptr<ioData> endPin;
		for (auto pin : loadedPins) {
			if (pin->getUniqueID() == endPinID) {
				endPin = pin;
				break;
			}
		}
		if (!endPin) return Logger::warn("Could not find matching node pin ID for end pin");

		if (!isConnectionValid(startPin, endPin)) return Logger::warn("Saved link could not be evaluated as a valid connection");
		std::shared_ptr<ioLink> newIoLink = std::make_shared<ioLink>();
		newIoLink->uniqueID = linkUniqueID;
		newIoLink->inputData = startPin->isOutput() ? startPin : endPin;
		newIoLink->outputData = endPin->isInput() ? endPin : startPin;
		startPin->ioLinks.push_back(newIoLink);
		endPin->ioLinks.push_back(newIoLink);
		loadedLinks.push_back(newIoLink);

		Logger::trace("Loaded Node Link with ID: {}  StartPin: {}  EndPin: {}", linkUniqueID, startPinID, endPinID);
		linkXML = linkXML->NextSiblingElement("Link");
	}

	ioNodeList.swap(loadedNodes);
	ioDataList.swap(loadedPins);
	ioLinkList.swap(loadedLinks);
	uniqueID = largestUniqueID + 1;
	Logger::debug("Largest unique ID is {}", largestUniqueID);
	Logger::info("Successfully loaded Node Graph");

	return true;
}
