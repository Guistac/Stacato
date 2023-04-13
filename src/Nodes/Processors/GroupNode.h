#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Environnement/NodeGraph/NodeGraph.h"
#include <tinyxml2.h>

class GroupNode : public Node {
public:																	
	
	DEFINE_NODE(GroupNode, Node::Type::CONTAINER , "Utility");
	
	glm::vec2 containerSize = glm::vec2(100.0, 100.0);
	
	virtual void restoreSavedPosition() override{
		Node::restoreSavedPosition();
		
	};
	
	virtual bool onSerialization() override{
		bool success = true;
		success &= Node::onSerialization();
		success &= serializeAttribute("Width", containerSize.x);
		success &= serializeAttribute("Height", containerSize.y);
		return success;
	}
	
	virtual bool onDeserialization() override{
		bool success = true;
		success &= Node::onDeserialization();
		success &= deserializeAttribute("Width", containerSize.x);
		success &= deserializeAttribute("Height", containerSize.y);
		return success;
	}
	
	virtual void onConstruction() override {
		Node::onConstruction();
		setName("Group");
	}

	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Node::onCopyFrom(source);
	}
	
};

