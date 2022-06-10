#include <pch.h>

#include "Widgets.h"
#include "Dashboard.h"

#include <tinyxml2.h>

void Widget::gui(){}

void Widget::addToDictionnary(){ WidgetManager::addToDictionnary(shared_from_this()); }

void Widget::removeFromDictionnary(){ WidgetManager::removeFromDictionnary(shared_from_this()); }

std::shared_ptr<WidgetInstance> WidgetInstance::make(std::shared_ptr<Widget> widget){
	auto instance = std::make_shared<WidgetInstance>();
	instance->widget = widget;
	instance->size = widget->getDefaultSize();
	instance->uniqueID = widget->uniqueID;
	return instance;
}

bool WidgetInstance::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	xml->SetAttribute("UniqueID", uniqueID);
	xml->SetAttribute("PositionX", position.x);
	xml->SetAttribute("PositionY", position.y);
	xml->SetAttribute("SizeX", size.x);
	xml->SetAttribute("SizeY", size.y);
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
	return widgetInstance;
}



namespace WidgetManager{

	int uniqueIdCounter = 0;

	int getNewUniqueID(){
		uniqueIdCounter++;
		return uniqueIdCounter;
	}
	void registerUniqueID(int registeredID){
		if(registeredID > uniqueIdCounter) uniqueIdCounter = registeredID;
	}

	std::vector<std::shared_ptr<Widget>> dictionnary;
	std::vector<std::shared_ptr<Widget>>& getDictionnary(){ return dictionnary; }

	void addToDictionnary(std::shared_ptr<Widget> widget){
		if(widget->uniqueID == -1) widget->uniqueID = getNewUniqueID();
		dictionnary.push_back(widget);
		for(auto& dashboard : DashboardManager::getDashboards()){
			dashboard->addAvailableWidget(widget);
		}
	}
	void removeFromDictionnary(std::shared_ptr<Widget> widget){
		for(int i = 0; i < dictionnary.size(); i++){
			if(dictionnary[i] == widget){
				dictionnary.erase(dictionnary.begin() + i);
				break;
			}
		}
		for(auto& dashboard : DashboardManager::getDashboards()){
			dashboard->removeAvailableWidget(widget);
		}
	}

	std::shared_ptr<Widget> getWidgetByUniqueID(int uniqueID){
		for(auto& widget : dictionnary){
			if(widget->uniqueID == uniqueID) return widget;
		}
		return nullptr;
	}



}
