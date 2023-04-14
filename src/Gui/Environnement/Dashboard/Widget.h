#pragma once

namespace tinyxml2{ struct XMLElement; }

class Widget : public std::enable_shared_from_this<Widget>{
public:
	
	std::string category = "None";
	int uniqueID = -1;
	
	virtual std::string getName() = 0;
	
	Widget(std::string category_) : category(category_){}
		
	virtual void gui() = 0;
	
	virtual std::shared_ptr<Widget> makeCopy(){ return nullptr; }
	
	void addToDictionnary();
	void removeFromDictionnary();
};


class WidgetInstance{
public:
	
	glm::vec2 position;
	glm::vec2 size;
	std::shared_ptr<Widget> widget = nullptr;
	int uniqueID = -1;
	
	static std::shared_ptr<WidgetInstance> make(std::shared_ptr<Widget> widget);
		
	bool hasWidget(){ return widget != nullptr; }
	std::shared_ptr<Widget> getWidget(){ return widget; }
	void gui();
	
	bool save(tinyxml2::XMLElement* xml);
	static std::shared_ptr<WidgetInstance> load(tinyxml2::XMLElement* xml);
	
};
