#include <pch.h>

#include "Node.h"

#include "NodeGraph.h"
#include "NodePin.h"

void Node::onConstruction() {
	Component::onConstruction();
	setSaveString("Node");
	inputPins = Legato::ListComponent<NodePin>::createInstance();
	inputPins->setSaveString("InputPins");
	inputPins->setEntrySaveString("Pin");
	outputPins = Legato::ListComponent<NodePin>::createInstance();
	outputPins->setSaveString("OutputPins");
	outputPins->setEntrySaveString("Pin");
}

void Node::onCopyFrom(std::shared_ptr<PrototypeBase> source) {
	Component::onCopyFrom(source);
	
}

void Node::addNodePin(std::shared_ptr<NodePin> pin) {

	//don't add the nodepin if the node already has it
	for (auto inputPin : inputPins->getEntries()) if (inputPin == pin) return;
	for (auto outputPin : outputPins->getEntries()) if (outputPin == pin) return;

	if (pin->isInput()) inputPins->addEntry(pin);
	else if (pin->isOutput()) outputPins->addEntry(pin);

	//if a pins gets added after if the node is already on the editor, this handles everything
	if (nodeGraph != nullptr) {
		pin->uniqueID = nodeGraph->getNewUniqueID();
		pin->parentNode = std::static_pointer_cast<Node>(shared_from_this());
	}
}

void Node::removeNodePin(std::shared_ptr<NodePin> pin) {
	if (pin->isInput()) inputPins->removeEntry(pin);
	else if (pin->isOutput()) outputPins->removeEntry(pin);
	pin->disconnectAllLinks();
	pin->parentNode = nullptr;
}

//check if all nodes linked to the inputs of this node were processed
bool Node::areAllLinkedInputNodesProcessed() {
	for (auto inputData : inputPins->getEntries()) {
		for (auto inputDataLink : inputData->getLinks()) {
			auto connectedNode = inputDataLink->getInputPin()->getNode();
			if (!connectedNode->wasProcessed()) return false;
		}
	}
	return true;
}

bool Node::areAllLinkedBidirectionalOutputNodesProcessed(){
	for(auto outputData : outputPins->getEntries()){
		if(!outputData->isBidirectional()) continue;
		for(auto outputDataLink : outputData->getLinks()){
			auto connectedNode = outputDataLink->getOutputPin()->getNode();
			if(!connectedNode->wasProcessed()) return false;
		}
	}
	return true;
}

bool Node::onSerialization() {
	Component::onSerialization();
	
	serializeAttribute("UniqueID", getUniqueID());
	serializeAttribute("ClassName", getClassName());
	serializeAttribute("NodeType", Enumerator::getSaveString(getType()));
	
	/*
	if(getType() == Type::IODEVICE){
		auto thisDevice = std::static_pointer_cast<Device>(shared_from_this());
		serializeAttribute("DeviceType", getDeviceType(thisDevice->getDeviceType())->saveName);
	}
	*/
	 
	Serializable positionXML;
	positionXML.setSaveString("NodeGraphPosition");
	positionXML.serializeIntoParent(this);
	positionXML.serializeAttribute("Split", isSplit());
	if(isSplit()){
		glm::vec2 inputPos, outputPos;
		getSplitNodeGraphPosition(inputPos, outputPos);
		positionXML.serializeAttribute("inputX", inputPos.x);
		positionXML.serializeAttribute("inputY", inputPos.y);
		positionXML.serializeAttribute("outputX", outputPos.x);
		positionXML.serializeAttribute("outputY", outputPos.y);
	}else{
		glm::vec2 pos = getNodeGraphPosition();
		positionXML.serializeAttribute("X", getNodeGraphPosition().x);
		positionXML.serializeAttribute("Y", getNodeGraphPosition().y);
	}
	
	inputPins->serializeIntoParent(this);
	outputPins->serializeIntoParent(this);
	
	return true;
}

bool Node::onDeserialization() {
	Component::onDeserialization();
	
	deserializeAttribute("UniqueID", uniqueID);
	
	Serializable positionSerializable;
	positionSerializable.setSaveString("NodeGraphPosition");
	positionSerializable.deserializeFromParent(this);
	
	std::string nodeTypeString;
	deserializeAttribute("NodeType", nodeTypeString);
	if(Enumerator::isValidSaveName<Node::Type>(nodeTypeString.c_str())){
		Node::Type nodeType = Enumerator::getEnumeratorFromSaveString<Node::Type>(nodeTypeString.c_str());
		if(nodeType == Node::Type::IODEVICE) positionSerializable.deserializeAttribute("Split", b_isSplit);
	}
	
	if(isSplit()){
		positionSerializable.deserializeAttribute("inputX", savedPosition.x);
		positionSerializable.deserializeAttribute("inputY", savedPosition.y);
		positionSerializable.deserializeAttribute("outputX", savedSplitPosition.x);
		positionSerializable.deserializeAttribute("outputY", savedSplitPosition.y);
	}else{
		positionSerializable.deserializeAttribute("X", savedPosition.x);
		positionSerializable.deserializeAttribute("Y", savedPosition.y);
	}
	
	//after abstract node deserialization,
	//the node implementation loads its own data (with eventual dynamically created pins)
	//the node adds all its pins to the pin lists
	
}

bool Node::loadPins(){
	
	//load all pins into temporary lists
	std::shared_ptr<Legato::ListComponent<NodePin>> abstractInputPins = Legato::ListComponent<NodePin>::createInstance();
	std::shared_ptr<Legato::ListComponent<NodePin>> abstractOutputPins = Legato::ListComponent<NodePin>::createInstance();
	abstractInputPins->setSaveString("InputPins");
	abstractInputPins->setEntrySaveString("Pin");
	abstractInputPins->setEntryConstructor([](Legato::Serializable& abstractEntry){ return NodePin::createInstance(); });
	abstractOutputPins->setSaveString("OutputPins");
	abstractOutputPins->setEntrySaveString("Pin");
	abstractOutputPins->setEntryConstructor([](Legato::Serializable& abstractEntry){ return NodePin::createInstance(); });
	abstractInputPins->deserializeFromParent(this);
	abstractOutputPins->deserializeFromParent(this);
	
	//match the temporary pins with pins create or loaded by the node
	//and transfer all inforation to the node pins
	for(auto pin : inputPins->getEntries()){
		for(auto abstractPin : abstractInputPins->getEntries()){
			if(pin->getSaveString() != abstractPin->getSaveString()) continue;
			if(pin->dataType != abstractPin->dataType) continue;
			pin->setName(abstractPin->getName());
			pin->uniqueID = abstractPin->uniqueID;
			pin->b_visible = abstractPin->b_visible;
			//copy more fields and also make this into a lambda to avoid repetition ??
		}
		return false;
	}
	for(auto pin : outputPins->getEntries()){
		for(auto abstractPin : abstractOutputPins->getEntries()){
			if(pin->getSaveString() != abstractPin->getSaveString()) continue;
			if(pin->dataType != abstractPin->dataType) continue;
			pin->setName(abstractPin->getName());
			pin->uniqueID = abstractPin->uniqueID;
			pin->b_visible = abstractPin->b_visible;
		}
		return false;
	}
	
	//Register loaded unique ids with nodegraph so it always has the highest id
	
}
