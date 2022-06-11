#pragma once

#include "Project/Editor/Parameter.h"

namespace tinyxml2{ struct XMLElement; }

class Widget : public std::enable_shared_from_this<Widget>{
public:
	
	std::string name;
	std::string category = "None";
	int uniqueID = -1;
	
	Widget(std::string name_, std::string category_) : name(name_), category(category_){}
	
	virtual void gui();
	virtual bool isResizeable(){ return false; }
	virtual glm::vec2 getDefaultSize(){ return {256.0, 256.0}; }
	
	virtual std::shared_ptr<Widget> makeCopy(){ return nullptr; }
	
	void addToDictionnary();
	void removeFromDictionnary();
};


class WidgetInstance{
public:
	
	glm::vec2 position;
	glm::vec2 size;
	std::shared_ptr<Widget> widget;
	int uniqueID = -1;
	
	std::shared_ptr<VectorParameter<glm::vec2>> positionParameter = std::make_shared<VectorParameter<glm::vec2>>(glm::vec2(0,0), "Position", "Position");
	std::shared_ptr<VectorParameter<glm::vec2>> sizeParameter = std::make_shared<VectorParameter<glm::vec2>>(glm::vec2(0,0), "Size", "Size");
	
	static std::shared_ptr<WidgetInstance> make(std::shared_ptr<Widget> widget);
	
	void gui(){
		if(widget) widget->gui();
	}
	bool isResizeable(){
		if(widget) return widget->isResizeable();
		return false;
	}
	glm::vec2 getDefaultSize(){
		if(widget) return widget->getDefaultSize();
		return size;
	}
	
	bool save(tinyxml2::XMLElement* xml);
	static std::shared_ptr<WidgetInstance> load(tinyxml2::XMLElement* xml);
	
};
