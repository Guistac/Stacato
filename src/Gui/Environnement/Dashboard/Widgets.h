#pragma once

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


namespace WidgetManager{

	int getNewUniqueID();
	void registerUniqueID(int uniqueID);
	
	std::vector<std::shared_ptr<Widget>>& getDictionnary();
	void addToDictionnary(std::shared_ptr<Widget> widget);
	void removeFromDictionnary(std::shared_ptr<Widget> widget);

	std::shared_ptr<Widget> getWidgetByUniqueID(int uniqueID);
};
