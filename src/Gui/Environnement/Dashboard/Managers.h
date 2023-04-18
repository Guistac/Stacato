#pragma once

#include "Legato/Gui/Window.h"

namespace tinyxml2{ struct XMLElement; }
class Dashboard;
class Widget;

namespace DashboardManager{
	
	//std::vector<std::shared_ptr<Dashboard>>& getDashboards();
	std::shared_ptr<Dashboard> getDashboard();

	bool save(tinyxml2::XMLElement* xml);
	bool load(tinyxml2::XMLElement* xml);

};

class DashboardWindow : public Legato::Window{
public:
	DashboardWindow() : Window("Dashboard", false){}
	virtual void onDraw() override;
	SINGLETON_GET_METHOD(DashboardWindow);
};

namespace WidgetManager{

	int getNewUniqueID();
	void registerUniqueID(int uniqueID);
	
	std::vector<std::shared_ptr<Widget>>& getDictionnary();

	void addToDictionnary(std::shared_ptr<Widget> widget);
	void removeFromDictionnary(std::shared_ptr<Widget> widget);

	std::shared_ptr<Widget> getWidgetByUniqueID(int uniqueID);
};
