#include <pch.h>

#include "Widget.h"
#include "Managers.h"
#include "Dashboard.h"

#include <tinyxml2.h>

namespace DashboardManager{

std::vector<std::shared_ptr<Dashboard>> dashboards = {
	std::make_shared<Dashboard>()
};
std::vector<std::shared_ptr<Dashboard>>& getDashboards(){ return dashboards; }

bool save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	for(auto& dashboard : dashboards){
		XMLElement* dashboardXML = xml->InsertNewChildElement("Dashboard");
		dashboard->save(dashboardXML);
	}
	return true;
}
bool load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;

	XMLElement* dashboardXML = xml->FirstChildElement("Dashboard");
	while(dashboardXML){
		auto dashboard = Dashboard::load(dashboardXML);
		if(dashboard == nullptr){
			Logger::warn("Error Loading Dashboard");
			return false;
		}
		dashboards.push_back(dashboard);
		dashboardXML = dashboardXML->NextSiblingElement("Dashboard");
	}
	return true;
}


};




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
