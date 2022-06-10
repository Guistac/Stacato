#include <pch.h>

#include "Widgets.h"
#include "Dashboard.h"

void Widget::gui(){}

void Widget::addToDictionnary(){ WidgetManager::addToDictionnary(shared_from_this()); }

void Widget::removeFromDictionnary(){ WidgetManager::removeFromDictionnary(shared_from_this()); }




namespace WidgetManager{

	int uniqueIdCounter = 0;
	int getNewUniqueID(){ return uniqueIdCounter++; }

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
