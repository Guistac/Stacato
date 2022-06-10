#pragma once


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
	
	static std::shared_ptr<WidgetInstance> make(std::shared_ptr<Widget> widget){
		auto instance = std::make_shared<WidgetInstance>(widget);
		instance->size = widget->getDefaultSize();
		return instance;
	}
	
	WidgetInstance(std::shared_ptr<Widget> widget_) : widget(widget_){}
	
	void gui(){ widget->gui(); }
	bool isResizeable(){ return widget->isResizeable(); }
	glm::vec2 getDefaultSize(){ return widget->getDefaultSize(); }
	
};


namespace WidgetManager{

	int getNewUniqueID();
	
	std::vector<std::shared_ptr<Widget>>& getDictionnary();
	void addToDictionnary(std::shared_ptr<Widget> widget);
	void removeFromDictionnary(std::shared_ptr<Widget> widget);

	std::shared_ptr<Widget> getWidgetByUniqueID(int uniqueID);
};
