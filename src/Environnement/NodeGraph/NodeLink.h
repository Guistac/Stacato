#pragma once

#include "Legato/Editor/Component.h"
#include "NodePin.h"

class NodePin;
class NodeGraph;

class NodeLink : public Legato::Component{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(NodeLink)
	
public:
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
public:

	int getUniqueID() { return uniqueID; }

	std::shared_ptr<NodePin> getInputPin() { return outputPin; }
	std::shared_ptr<NodePin> getOutputPin() { return inputPin; }

	std::shared_ptr<NodePin> outputPin = nullptr;
	std::shared_ptr<NodePin> inputPin = nullptr;

	int uniqueID = -1;
	std::shared_ptr<NodeGraph> nodeGraph = nullptr;
	
	void disconnect();
};

