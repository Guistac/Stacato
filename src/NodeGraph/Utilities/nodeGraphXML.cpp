#include <pch.h>

#include "NodeGraph/NodeGraph.h"

#include "NodeFactory.h"
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
			std::shared_ptr<DeviceNode> device = std::dynamic_pointer_cast<DeviceNode>(node);
			nodeXML->SetAttribute("DeviceType", device->getDeviceTypeString());
			nodeXML->SetAttribute("Split", node->isSplit());
		}
		nodeXML->SetAttribute("CustomName", node->getName());

		if (!node->isSplit()) {
			XMLElement* positionXML = nodeXML->InsertNewChildElement("NodeEditorPosition");
			glm::vec2 position = node->getNodeGraphPosition();
			positionXML->SetAttribute("x", position.x);
			positionXML->SetAttribute("y", position.y);
		}
		else {
			XMLElement* inputPositionXML = nodeXML->InsertNewChildElement("InputNodeEditorPosition");
			XMLElement* outputPositionXML = nodeXML->InsertNewChildElement("OutputNodeEditorPosition");
			glm::vec2 input, output;
			node->getSplitNodeGraphPosition(input, output);
			inputPositionXML->SetAttribute("x", input.x);
			inputPositionXML->SetAttribute("y", input.y);
			outputPositionXML->SetAttribute("x", output.x);
			outputPositionXML->SetAttribute("y", output.y);
		}

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

	std::vector<std::shared_ptr<Node>> loadedNodes;
	std::vector<std::shared_ptr<NodePin>> loadedPins;
	std::vector<std::shared_ptr<NodeLink>> loadedLinks;
	int largestUniqueID = 0; //TODO: need to increment unique id counter to found biggest value so we can add more elements after loading

	XMLElement* nodeXML = nodesXML->FirstChildElement("Node");
	while (nodeXML) {

		//Load General Node Attributes
		const char* className;
		if (nodeXML->QueryStringAttribute("ClassName", &className) != XML_SUCCESS) return Logger::warn("Could not load Node ClassName");
		int nodeUniqueID;
		if (nodeXML->QueryIntAttribute("UniqueID", &nodeUniqueID) != XML_SUCCESS) return Logger::warn("Could not load Node ID");
		if (nodeUniqueID > largestUniqueID) largestUniqueID = nodeUniqueID;
		const char* nodeCustomName;
		if (nodeXML->QueryStringAttribute("CustomName", &nodeCustomName) != XML_SUCCESS) return Logger::warn("Could not load Node Custom Name");
		const char* nodeType;
		if (nodeXML->QueryStringAttribute("NodeType", &nodeType) != XML_SUCCESS) return Logger::warn("Could not load Node Type");
		bool isSplit = false;
		if (strcmp(nodeType, "IODEVICE") == 0) {
			if (nodeXML->QueryBoolAttribute("Split", &isSplit) != XML_SUCCESS) return Logger::warn("Could not load split status");
		}

		//Construct Node Object from Type Attributes
		Logger::trace("Loading node '{}'", className);
		std::shared_ptr<Node> loadedNode = nullptr;
		if (strcmp(nodeType, "PROCESSOR") == 0) loadedNode = NodeFactory::getIoNodeByName(className);
		else if (strcmp(nodeType, "CLOCK") == 0) loadedNode = NodeFactory::getIoNodeByName(className);
		else if (strcmp(nodeType, "IODEVICE") == 0) {
			const char* deviceType;
			if (nodeXML->QueryStringAttribute("DeviceType", &deviceType) != XML_SUCCESS) return Logger::warn("Could not load Node Device Type");
			if (strcmp(deviceType, "ETHERCATSLAVE") == 0) loadedNode = EtherCatDeviceFactory::getDeviceByName(className);
		}
		else if (strcmp(nodeType, "AXIS") == 0) loadedNode = NodeFactory::getAxisByName(className);
		else if (strcmp(nodeType, "CONTAINER") == 0) loadedNode = NodeFactory::getIoNodeByName(className);
		if (loadedNode == nullptr) return Logger::warn("Coult not load Node Class");
	
		//Get Node position in node editor
		if (!isSplit) {
			XMLElement* positionXML = nodeXML->FirstChildElement("NodeEditorPosition");
			if (!positionXML) return Logger::warn("Could not load node position attribute");
			float xPosition, yPosition;
			positionXML->QueryFloatAttribute("x", &xPosition);
			positionXML->QueryFloatAttribute("y", &yPosition);
			loadedNode->savedPosition = glm::vec2(xPosition, yPosition);
		}
		else {
			XMLElement* inputPositionXML = nodeXML->FirstChildElement("InputNodeEditorPosition");
			if (!inputPositionXML) return Logger::warn("Could not load split input node position attribute");
			float xInput, yInput;
			inputPositionXML->QueryFloatAttribute("x", &xInput);
			inputPositionXML->QueryFloatAttribute("y", &yInput);
			XMLElement* outputPositionXML = nodeXML->FirstChildElement("OutputNodeEditorPosition");
			if (!outputPositionXML) return Logger::warn("Could not load split output node position attribute");
			float xOutput, yOutput;
			outputPositionXML->QueryFloatAttribute("x", &xOutput);
			outputPositionXML->QueryFloatAttribute("y", &yOutput);
			loadedNode->savedPosition = glm::vec2(xInput, yInput);
			loadedNode->savedSplitPosition = glm::vec2(xOutput, yOutput);
		}

		//assign node data, creates default node pins and adds them to the node
		loadedNode->setName(nodeCustomName);
		loadedNode->assignIoData(); //this assigns a unique id if the parent nodegraph is assigned, we don't want that
		loadedNode->uniqueID = nodeUniqueID;
		loadedNode->parentNodeGraph = this;
		loadedNode->b_isSplit = isSplit;
		loadedNode->b_wasSplit = isSplit;
		loadedNodes.push_back(loadedNode);
		loadedNode->b_isInNodeGraph = true;
		for (std::shared_ptr<NodePin> data : loadedNode->nodeInputData) {
			data->parentNode = loadedNode;
			loadedPins.push_back(data);
		}
		for (std::shared_ptr<NodePin> data : loadedNode->nodeOutputData) {
			data->parentNode = loadedNode;
			loadedPins.push_back(data);
		}

		//loads node specific data, creates modular node pins
		XMLElement* nodeSpecificDataXML = nodeXML->FirstChildElement("NodeSpecificData");
		if (!nodeSpecificDataXML) return Logger::warn("Could not load Node Specific Data");
		if (!loadedNode->load(nodeSpecificDataXML)) return Logger::warn("Could not read node specific data");

		//load pin information, matches pins by SaveName and DataType, then assigns unique ids, data and other information
		XMLElement* inputPinsXML = nodeXML->FirstChildElement("InputPins");
		if (!inputPinsXML) return Logger::warn("Could Not Load Node InputPins");
		XMLElement* inputPinXML = inputPinsXML->FirstChildElement("InputPin");
		while (inputPinXML) {
			const char* saveNameString;
			const char* dataTypeString;
			if (inputPinXML->QueryStringAttribute("SaveName", &saveNameString) != XML_SUCCESS) return Logger::warn("Could not load Input Pin SaveName Attribute");
			if (inputPinXML->QueryStringAttribute("DataType", &dataTypeString) != XML_SUCCESS) return Logger::warn("Could not load Input Pin DataType Attribute");
			std::shared_ptr<NodePin> matchingPin = nullptr;
			for (auto pin : loadedNode->getNodeInputData()) {
				if (pin->matches(saveNameString, dataTypeString)) {
					if (!pin->load(inputPinXML)) return Logger::warn("Could not load Node Pin '{}'", saveNameString);
					matchingPin = pin;
					break;
				}
			}
			if (matchingPin == nullptr) return Logger::warn("Could not find pin Matching name: {}  datatype: {}", saveNameString, dataTypeString);
			if (matchingPin->getUniqueID() > largestUniqueID) largestUniqueID = matchingPin->getUniqueID();
			Logger::trace("Loaded Input Pin {} (DisplayName: '{}') with dataType: {} visibility: {}", matchingPin->getSaveName(), matchingPin->getDisplayName(), matchingPin->getTypeName(), matchingPin->isVisible());
			inputPinXML = inputPinXML->NextSiblingElement();
		}
		XMLElement* outputPinsXML = nodeXML->FirstChildElement("OutputPins");
		if (!outputPinsXML) return Logger::warn("Could Not Load Node OutputPins");
		XMLElement* outputPinXML = outputPinsXML->FirstChildElement("OutputPin");
		while (outputPinXML) {
			const char* saveNameString;
			const char* dataTypeString;
			if (outputPinXML->QueryStringAttribute("SaveName", &saveNameString) != XML_SUCCESS) return Logger::warn("Could not load Output Pin SaveName Attribute");
			if (outputPinXML->QueryStringAttribute("DataType", &dataTypeString) != XML_SUCCESS) return Logger::warn("Could not load Output Pin DataType Attribute");
			std::shared_ptr<NodePin> matchingPin = nullptr;
			for (auto pin : loadedNode->getNodeOutputData()) {
				if (pin->matches(saveNameString, dataTypeString)) {
					if (!pin->load(outputPinXML)) return Logger::warn("Could not load Node Pin '{}'", saveNameString);
					matchingPin = pin;
					break;
				}
			}
			if (matchingPin == nullptr) return Logger::warn("Could not find pin Matching name: {}  datatype: {}", saveNameString, dataTypeString);
			if (matchingPin->getUniqueID() > largestUniqueID) largestUniqueID = matchingPin->getUniqueID();
			Logger::trace("Loaded Output Pin {} (DisplayName: '{}') with dataType: {} visibility: {}", matchingPin->getSaveName(), matchingPin->getDisplayName(), matchingPin->getTypeName(), matchingPin->isVisible());
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

		std::shared_ptr<NodePin> startPin;
		for (auto pin : loadedPins) {
			if (pin->getUniqueID() == startPinID) {
				startPin = pin;
				break;
			}
		}
		if (!startPin) return Logger::warn("Could not find matching node pin ID for start pin");
		std::shared_ptr<NodePin> endPin;
		for (auto pin : loadedPins) {
			if (pin->getUniqueID() == endPinID) {
				endPin = pin;
				break;
			}
		}
		if (!endPin) return Logger::warn("Could not find matching node pin ID for end pin");

		if (!isConnectionValid(startPin, endPin)) return Logger::warn("Saved link could not be evaluated as a valid connection");
		std::shared_ptr<NodeLink> newIoLink = std::make_shared<NodeLink>();
		newIoLink->uniqueID = linkUniqueID;
		newIoLink->inputData = startPin->isOutput() ? startPin : endPin;
		newIoLink->outputData = endPin->isInput() ? endPin : startPin;
		startPin->NodeLinks.push_back(newIoLink);
		endPin->NodeLinks.push_back(newIoLink);
		loadedLinks.push_back(newIoLink);

		Logger::trace("Loaded Node Link with ID: {}  StartPin: {}  EndPin: {}", linkUniqueID, startPinID, endPinID);
		linkXML = linkXML->NextSiblingElement("Link");
	}

	NodeList.swap(loadedNodes);
	NodePinList.swap(loadedPins);
	NodeLinkList.swap(loadedLinks);
	uniqueID = largestUniqueID + 1; //set this so we can add more elements to the node graph after loading

	Logger::debug("Largest unique ID is {}", largestUniqueID);
	Logger::info("Successfully loaded Node Graph");

	b_justLoaded = true;

	return true;
}
