#pragma once

namespace Dashboard{

	class Widget{
	public:
		
		glm::vec2 position;
		glm::vec2 size;
		
		virtual void gui() = 0;
		virtual bool isResizeable(){ return false; }
		
	};

	std::vector<std::shared_ptr<Widget>>& getWidgets();

	void addWidget(std::shared_ptr<Widget> widget);
	void removeWidget(std::shared_ptr<Widget> widget);
	void moveWidget(glm::vec2 newPosition);
	void resizeWidget(glm::vec2 newSize);

	void fitView();

};
