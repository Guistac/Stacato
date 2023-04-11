#include <pch.h>

#include "Nodes/NodeFactory.h"
#include "Environnement/Environnement.h"
#include "Environnement/NodeGraph/NodeGraph.h"
#include "Gui/Environnement/NodeGraph/NodeGraphGui.h"

#include <tinyxml2.h>

#include "Legato/Editor/SerializableList.h"

bool NodeGraph::onSerialization(){
	Component::onSerialization();
	
	//this needs to be called to satisfy the node editor library
	//if we never displayed the node editor, this means we never submitted any nodes to the library
	//and we can't retrieve information such as node positions from the library
	//we submit all node once here before saving so the library is happy
	//we can proceed as if the node editor had been displayed all along
	//if the node editor was displayed once this is useless but we submit the draw call anyways since it is offscreen
	ImGui::SetNextWindowPos(ImVec2(10000, 10000));
	ImGui::Begin("OffscreenNodeEditorFor1Frame", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
	editorGui(ImVec2(0,0));
	ImGui::End();
	
	nodeList->serializeIntoParent(this);
	linkList->serializeIntoParent(this);
	
	return true;
}

bool NodeGraph::onDeserialization(){
	Component::onDeserialization();

	/*
	Logger::debug("Loading Nodegraph Data");

	XMLElement* nodesXML = xml->FirstChildElement("Nodes");
	if (!nodesXML) return Logger::warn("Could not load Nodes element from savefile");

	int largestUniqueID = 0;

	XMLElement* nodeXML = nodesXML->FirstChildElement("Node");
	while (nodeXML) {

		//Load General Node Attributes
		const char* nodeSaveNameString;
		if (nodeXML->QueryStringAttribute("ClassName", &nodeSaveNameString) != XML_SUCCESS) return Logger::warn("Could not load Node ClassName");
		int nodeUniqueID;
		if (nodeXML->QueryIntAttribute("UniqueID", &nodeUniqueID) != XML_SUCCESS) return Logger::warn("Could not load Node ID");
		if (nodeUniqueID > largestUniqueID) largestUniqueID = nodeUniqueID;
		const char* nodeCustomName;
		if (nodeXML->QueryStringAttribute("CustomName", &nodeCustomName) != XML_SUCCESS) return Logger::warn("Could not load Node Custom Name");
		const char* nodeTypeString;
		if (nodeXML->QueryStringAttribute("NodeType", &nodeTypeString) != XML_SUCCESS) return Logger::warn("Could not load Node Type");
		if (!Enumerator::isValidSaveName<Node::Type>(nodeTypeString)) return Logger::warn("Could not read node type");
		Node::Type nodeType = Enumerator::getEnumeratorFromSaveString<Node::Type>(nodeTypeString);

		//Construct Node Object from Type Attributes
		//on construction initialize() is called to assign node data, creates static/static node pins and adds them to the nodes input and output data lists
		//no unique ids are loaded or generated here
		Logger::trace("Loading node '{}' of type '{}'", nodeCustomName, nodeSaveNameString);
		std::shared_ptr<Node> loadedNode = nullptr;
		bool isSplit = false;
		
		if(nodeType == Node::Type::IODEVICE){
			const char* deviceTypeString;
			if (nodeXML->QueryBoolAttribute("Split", &isSplit) != XML_SUCCESS) return Logger::warn("Could not load split status");
		}
		
		loadedNode = NodeFactory::getNodeBySaveName(nodeSaveNameString);
		if (loadedNode == nullptr) return Logger::warn("Coult not load Node Class");
		
		//Get Node position in node editor
		if (!isSplit) {
			XMLElement* positionXML = nodeXML->FirstChildElement("NodeEditorPosition");
			if (!positionXML) return Logger::warn("Could not load node position attribute");
			float xPosition, yPosition;
			positionXML->QueryFloatAttribute("x", &xPosition);
			positionXML->QueryFloatAttribute("y", &yPosition);
			//xPosition *= ApplicationWindow::getScaleTuning();
			//yPosition *= ApplicationWindow::getScaleTuning();
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
			//xInput *= ApplicationWindow::getScaleTuning();
			//yInput *= ApplicationWindow::getScaleTuning();
			//xOutput *= ApplicationWindow::getScaleTuning();
			//yOutput *= ApplicationWindow::getScaleTuning();
			loadedNode->savedPosition = glm::vec2(xInput, yInput);
			loadedNode->savedSplitPosition = glm::vec2(xOutput, yOutput);
		}

		//load node specific XML data
		//this also loads dynamically created pins and adds them to the nods input and output data lists
		//no unique ids are loaded or generated here
		XMLElement* nodeSpecificDataXML = nodeXML->FirstChildElement("NodeSpecificData");
		if (!nodeSpecificDataXML) return Logger::warn("Could not load Node Specific Data");
		if (!loadedNode->load(nodeSpecificDataXML)) return Logger::warn("Could not read node specific data");
		loadedNode->xmlElement = nodeSpecificDataXML;

		//Set General Node Data
		//All pins should be loaded now, so we build add them to the pin list of the node graph
		//set dependencies between node and nodegraph, between node and its pins
		loadedNode->setName(nodeCustomName);
		loadedNode->uniqueID = nodeUniqueID;
		loadedNode->b_isSplit = isSplit;
		loadedNode->b_wasSplit = isSplit;
		
		addNode(loadedNode);
		//Environnement::addNode(loadedNode);
		
		for (std::shared_ptr<NodePin> data : loadedNode->nodeInputPins) {
			data->parentNode = loadedNode;
			getPins().push_back(data);
		}
		for (std::shared_ptr<NodePin> data : loadedNode->nodeOutputPins) {
			data->parentNode = loadedNode;
			getPins().push_back(data);
		}

		//load pin data
		//matches loaded pins by SaveName and DataType, then assigns unique ids, data and other information
		XMLElement* inputPinsXML = nodeXML->FirstChildElement("InputPins");
		if (!inputPinsXML) return Logger::warn("Could Not Load Node InputPins");
		XMLElement* inputPinXML = inputPinsXML->FirstChildElement("InputPin");
		while (inputPinXML) {
			const char* saveNameString;
			if (inputPinXML->QueryStringAttribute("SaveString", &saveNameString) != XML_SUCCESS) return Logger::warn("Could not load Input Pin SaveName Attribute");
			const char* dataTypeString = "";
			inputPinXML->QueryStringAttribute("DataType", &dataTypeString);
			
			if(!Enumerator::isValidSaveName<NodePin::DataType>(dataTypeString)) return Logger::warn("Could not read Input Pin DataType Attribute");
			NodePin::DataType dataType = Enumerator::getEnumeratorFromSaveString<NodePin::DataType>(dataTypeString);
			std::shared_ptr<NodePin> matchingPin = nullptr;
			for (auto pin : loadedNode->getInputPins()) {
				if (pin->matches(saveNameString, dataType)) {
					if (!pin->load(inputPinXML)) return Logger::warn("Could not load Node Pin '{}'", saveNameString);
					matchingPin = pin;
					break;
				}
			}
			if (matchingPin == nullptr) {
				return Logger::warn("Could not find pin Matching name: {}  datatype: {}", saveNameString, dataTypeString);
			}
			if (matchingPin->getUniqueID() > largestUniqueID) largestUniqueID = matchingPin->getUniqueID();
			Logger::trace("Loaded Input Pin {} (DisplayName: '{}') with dataType: {} visibility: {}",
						  matchingPin->getSaveString(),
						  matchingPin->getDisplayString(),
						  Enumerator::getDisplayString(matchingPin->dataType),
						  matchingPin->isVisible());
			inputPinXML = inputPinXML->NextSiblingElement();
		}
		XMLElement* outputPinsXML = nodeXML->FirstChildElement("OutputPins");
		if (!outputPinsXML) return Logger::warn("Could Not Load Node OutputPins");
		XMLElement* outputPinXML = outputPinsXML->FirstChildElement("OutputPin");
		while (outputPinXML) {
			const char* saveNameString;
			if (outputPinXML->QueryStringAttribute("SaveString", &saveNameString) != XML_SUCCESS) return Logger::warn("Could not load Output Pin SaveName Attribute");
			const char* dataTypeString = "";
			outputPinXML->QueryStringAttribute("DataType", &dataTypeString);
			if (!Enumerator::isValidSaveName<NodePin::DataType>(dataTypeString)) return Logger::warn("Coul not read Output Pin DataType Attribute");
			NodePin::DataType dataType = Enumerator::getEnumeratorFromSaveString<NodePin::DataType>(dataTypeString);
			std::shared_ptr<NodePin> matchingPin = nullptr;
			for (auto pin : loadedNode->getOutputPins()) {
				if (pin->matches(saveNameString, dataType)) {
					if (!pin->load(outputPinXML)) return Logger::warn("Could not load Node Pin '{}'", saveNameString);
					matchingPin = pin;
					break;
				}
			}
			if (matchingPin == nullptr) return Logger::warn("Could not find pin Matching name: {}  datatype: {}", saveNameString, dataTypeString);
			if (matchingPin->getUniqueID() > largestUniqueID) largestUniqueID = matchingPin->getUniqueID();
			Logger::trace("Loaded Output Pin {} (DisplayName: '{}') with dataType: {} visibility: {}",
						  matchingPin->getSaveString(),
						  matchingPin->getDisplayString(),
						  Enumerator::getDisplayString(matchingPin->dataType),
						  matchingPin->isVisible());
			outputPinXML = outputPinXML->NextSiblingElement();
		}

		Logger::trace("Loaded Node {} (name: '{}') type: {}  ID: {}", nodeSaveNameString, nodeCustomName, nodeType, nodeUniqueID);
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
		for (auto pin : getPins()) {
			if (pin->getUniqueID() == startPinID) {
				startPin = pin;
				break;
			}
		}
		if (!startPin) return Logger::warn("Could not find matching node pin ID for start pin");
		
		std::shared_ptr<NodePin> endPin;
		for (auto pin : getPins()) {
			if (pin->getUniqueID() == endPinID) {
				endPin = pin;
				break;
			}
		}
		if (!endPin) return Logger::warn("Could not find matching node pin ID for end pin");

		if(!startPin->isConnectionValid(endPin)) return Logger::warn("Saved link could not be evaluated as a valid connection");
		
		std::shared_ptr<NodeLink> newIoLink = std::make_shared<NodeLink>();
		newIoLink->uniqueID = linkUniqueID;
		newIoLink->inputData = startPin->isOutput() ? startPin : endPin;
		newIoLink->outputData = endPin->isInput() ? endPin : startPin;
		startPin->nodeLinks.push_back(newIoLink);
		endPin->nodeLinks.push_back(newIoLink);
		
		getLinks().push_back(newIoLink);
		startPin->parentNode->onPinConnection(startPin);
		endPin->parentNode->onPinConnection(endPin);

		Logger::trace("Loaded Node Link with ID: {}  StartPin: {}  EndPin: {}", linkUniqueID, startPinID, endPinID);
		linkXML = linkXML->NextSiblingElement("Link");
	}
	
	for(auto node : nodes){
		if(!node->loadAfterLinksConnected(node->xmlElement)){
			Logger::warn("Could not load node {} after all links connected", node->getName());
		}
	}
	
	startCountingUniqueIDsFrom(largestUniqueID);
	
	Logger::trace("Largest unique ID is {}", largestUniqueID);
	Logger::info("Successfully loaded Node Graph");
*/
	
	return true;
}
