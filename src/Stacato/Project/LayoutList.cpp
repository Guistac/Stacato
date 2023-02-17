#include <pch.h>

#include "LayoutList.h"

#include <tinyxml2.h>

bool LayoutList::onSerialization() {
	using namespace tinyxml2;
	bool success = true;
	/*
	for(auto& layout : layouts){
		XMLElement* layoutXML = xmlElement->InsertNewChildElement("Layout");
		//success &= layout->save(layoutXML);
	}
	 */
	return success;
}

bool LayoutList::onDeserialization() {
	using namespace tinyxml2;
	
	layouts.deserializeFromParent(this);
	
	/*
	XMLElement* layoutXML = ->FirstChildElement("Layout");
	while(layoutXML){
		auto newLayout = Layout::load(layoutXML);
		if(newLayout == nullptr) {
			Logger::warn("Could not load layout");
			return false;
		}
		layouts.push_back(newLayout);
		layoutXML = layoutXML->NextSiblingElement("Layout");
	}
	*/
}

void LayoutList::onConstruction() {
	Component::onConstruction();
	setSaveString("LayoutList");
	layouts.setSaveString("Layouts");
}

void LayoutList::onCopyFrom(std::shared_ptr<PrototypeBase> source) {
	Component::onCopyFrom(source);
}
