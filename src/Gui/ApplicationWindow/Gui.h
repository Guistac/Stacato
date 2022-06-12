#pragma once

#include "Window.h"

namespace Gui{

	void initialize();
	void draw();

	std::vector<std::shared_ptr<Window>>& getWindowDictionnary();
	void addWindowToDictionnary(std::shared_ptr<Window> window);
	void removeWindowFromDictionnary(std::shared_ptr<Window> window);

	std::vector<std::shared_ptr<Window>>& getOpenWindows();
	void openWindow(std::shared_ptr<Window> window);

	void closeWindows();
	void closeAllWindows();

	std::vector<std::shared_ptr<Popup>>& getOpenPopups();
	void openPopup(std::shared_ptr<Popup> popup);
	void closePopup(std::shared_ptr<Popup> popup);
	
	void menuBar();
	void toolbar(float height);

	void resetToFactoryLayout();





	class QuitApplicationPopup : public Popup{
	public:
		QuitApplicationPopup() : Popup("Quit Application", true, true){}
		virtual void drawContent() override;
		SINGLETON_GET_METHOD(QuitApplicationPopup);
	};

	class AboutPopup : public Popup{
	public:
		AboutPopup() : Popup("About", true, true){}
		virtual void drawContent() override;
		SINGLETON_GET_METHOD(AboutPopup);
	};

}
