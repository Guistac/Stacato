#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Environnement/NodeGraph/NodeGraph.h"
#include <tinyxml2.h>

class GroupNode : public Node {
public:																	
	
	DEFINE_NODE(GroupNode, "Group", "Group", Node::Type::CONTAINER , "Utility");
	
	glm::vec2 containerSize = glm::vec2(100.0, 100.0);
	
	virtual void restoreSavedPosition() override{
		Node::restoreSavedPosition();
		
	};
	
	virtual bool load(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		XMLElement* containerXML = xml->FirstChildElement("Container");
		if(containerXML->QueryFloatAttribute("Width", &containerSize.x) != XML_SUCCESS){
			Logger::warn("Could not find attribute container Width");
			return false;
		}
		if(containerXML->QueryFloatAttribute("Height", &containerSize.x) != XML_SUCCESS){
			Logger::warn("Could not find attribute container Height");
			return false;
		}
		return true;
	}
	
	virtual bool save(tinyxml2::XMLElement* xml) override {
		using namespace tinyxml2;
		XMLElement* containerXML = xml->InsertNewChildElement("Container");
		containerXML->SetAttribute("Width", containerSize.x);
		containerXML->SetAttribute("Height", containerSize.y);
		return true;
	}
	
};

void GroupNode::initialize(){}
