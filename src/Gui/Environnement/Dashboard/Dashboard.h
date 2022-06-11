#pragma once

class Widget;
class WidgetInstance;
namespace tinyxml2{ struct XMLElement; }

class Dashboard{
public:
	
	void addWidget(std::shared_ptr<Widget> widget, glm::vec2 position);
	void removeWidget(std::shared_ptr<WidgetInstance> widget);
	void moveWidget(std::shared_ptr<WidgetInstance> widget, glm::vec2 newPosition);
	void resizeWidget(std::shared_ptr<WidgetInstance> widget, glm::vec2 newSize);

	void addAvailableWidget(std::shared_ptr<Widget> widget);
	void removeAvailableWidget(std::shared_ptr<Widget> widget);
	
	void fitView();
	
	void gui();
	void widgetAdder();
	void canvas();
	
	void selectWidget(std::shared_ptr<WidgetInstance> widget){ selectedWidget = widget; }
	void deselectWidget(){ selectedWidget = nullptr; }
	std::shared_ptr<WidgetInstance> getSelectedWidget(){ return selectedWidget; }
	bool isWidgetSelected(std::shared_ptr<WidgetInstance> widget){ return selectedWidget == widget; }

	std::vector<std::shared_ptr<WidgetInstance>>& getWidgets(){ return widgets; }
	
	std::vector<std::shared_ptr<WidgetInstance>> widgets;
	std::vector<std::shared_ptr<Widget>> availableWidgets;
	std::shared_ptr<WidgetInstance> selectedWidget = nullptr;
	std::shared_ptr<WidgetInstance> draggedWidget = nullptr;
	
	float scale = 1.0;
	float minScale = 0.1;
	float maxScale = 10.0;
	
	bool b_autoFit = false;
	bool b_drawGrid = true;
	bool b_lockWidgets = false;
	
	glm::vec2 offset;
	glm::vec2 dashboardPosition;
	glm::vec2 dashboardSize;
	glm::vec2 dashboardMax;
	
	void moveWidgetToTop(std::shared_ptr<WidgetInstance> widget);
	glm::vec2 screenToCanvas(glm::vec2 screen);
	glm::vec2 canvasToScreen(glm::vec2 canvas);
	glm::vec2 canvasToCursor(glm::vec2 canvas);
	void zoom(glm::vec2 screenZoomPosition, float delta);
	void pan(glm::vec2 mouseDelta);
	
	bool save(tinyxml2::XMLElement* xml);
	static std::shared_ptr<Dashboard> load(tinyxml2::XMLElement* xml);
};

