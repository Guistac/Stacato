#pragma once

class Window;
class Popup;

namespace Gui{

	void initialize();
	void draw();

	std::vector<std::shared_ptr<Window>>& getWindowDictionnary();
	void addWindowToDictionnary(std::shared_ptr<Window> window);
	void removeWindowFromDictionnary(std::shared_ptr<Window> window);

	std::vector<std::shared_ptr<Window>>& getOpenWindows();
	void openWindow(std::shared_ptr<Window> window);
	void closeWindow(std::shared_ptr<Window> window);

	std::vector<std::shared_ptr<Popup>>& getOpenPopups();
	void openPopup(std::shared_ptr<Popup> popup);
	void closePopup(std::shared_ptr<Popup> popup);
	






	void menuBar();
	void toolbar(float height);
	void popups();
	
	void quitApplicationPopup();
	
	void openAboutPopup();
	bool isAboutPopupOpenRequested();
	void aboutPopup();

	void resetDefaultLayout();
	bool shouldResetDefaultLayout();
	void finishResetDefaultLayout();

}
