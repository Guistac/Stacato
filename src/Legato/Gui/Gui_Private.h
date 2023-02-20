#pragma once

class Window;
class Popup;

namespace Legato::Gui{

void initialize();
void drawFrame();
void terminate();

};

namespace Legato::Gui::WindowManager{
	void update();

	void openWindow(std::shared_ptr<Window> window);
	void closeWindow(std::shared_ptr<Window> window);
	std::vector<std::shared_ptr<Window>>& getOpenWindows();

	void openPopup(std::shared_ptr<Popup> popup);
	void closePopup(std::shared_ptr<Popup> popup);
};
