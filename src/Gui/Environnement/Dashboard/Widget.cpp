#include <pch.h>

#include "Widget.h"
#include "Dashboard.h"
#include "Managers.h"

#include <tinyxml2.h>
#include <imgui.h>

void Widget::addToDictionnary(){ WidgetManager::addToDictionnary(shared_from_this()); }

void Widget::removeFromDictionnary(){ WidgetManager::removeFromDictionnary(shared_from_this()); }

std::shared_ptr<WidgetInstance> WidgetInstance::make(std::shared_ptr<Widget> widget){
	auto instance = std::make_shared<WidgetInstance>();
	instance->widget = widget;
	instance->uniqueID = widget->uniqueID;
	return instance;
}

bool WidgetInstance::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	if(widget != nullptr) xml->SetAttribute("Name", widget->getName().c_str());
	xml->SetAttribute("UniqueID", uniqueID);
	xml->SetAttribute("PositionX", position.x);
	xml->SetAttribute("PositionY", position.y);
	xml->SetAttribute("SizeX", size.x);
	xml->SetAttribute("SizeY", size.y);
	return true;
}

std::shared_ptr<WidgetInstance> WidgetInstance::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	auto widgetInstance = std::make_shared<WidgetInstance>();
	if(xml->QueryIntAttribute("UniqueID", &widgetInstance->uniqueID) != XML_SUCCESS) {
		Logger::warn("Could not find Widget Unique ID Attribute");
		return nullptr;
	}
	WidgetManager::registerUniqueID(widgetInstance->uniqueID);
	if(xml->QueryFloatAttribute("PositionX", &widgetInstance->position.x) != XML_SUCCESS) {
		Logger::warn("Could not find Widget Position X Attribute");
		return nullptr;
	}
	if(xml->QueryFloatAttribute("PositionY", &widgetInstance->position.y) != XML_SUCCESS) {
		Logger::warn("Could not find Widget Position Y Attribute");
		return nullptr;
	}
	if(xml->QueryFloatAttribute("SizeX", &widgetInstance->size.x) != XML_SUCCESS) {
		Logger::warn("Could not find Widget Size X Attribute");
		return nullptr;
	}
	if(xml->QueryFloatAttribute("SizeY", &widgetInstance->size.y) != XML_SUCCESS) {
		Logger::warn("Could not find Widget Size Y Attribute");
		return nullptr;
	}
	widgetInstance->position.x = std::round(widgetInstance->position.x);
	widgetInstance->position.y = std::round(widgetInstance->position.y);
	return widgetInstance;
}

void WidgetInstance::gui(){
	if(!widget) return;
	widget->gui();
}
