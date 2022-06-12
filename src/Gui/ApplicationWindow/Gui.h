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
	void closeWindow(std::shared_ptr<Window> window);

	void closeAllWindows();

	std::vector<std::shared_ptr<Popup>>& getOpenPopups();
	void openPopup(std::shared_ptr<Popup> popup);
	void closePopup(std::shared_ptr<Popup> popup);
	
	void menuBar();
	void toolbar(float height);

	void resetToFactoryLayout();


	void popups();
	



	class QuitApplicationPopup : public Popup{
	public:
		QuitApplicationPopup() : Popup("Quit Application", true, true){}
		bool b_quitApplication;
		virtual void drawContent() override;
		virtual void onClose() override;
		virtual void onOpen() override { b_quitApplication = false; };
		static std::shared_ptr<QuitApplicationPopup> get(){
			static std::shared_ptr<QuitApplicationPopup> popup = std::make_shared<QuitApplicationPopup>();
			return popup;
		}
	};

	class AboutPopup : public Popup{
	public:
		AboutPopup() : Popup("About", true, true){}
		virtual void drawContent() override;
		static std::shared_ptr<AboutPopup> get(){
			static std::shared_ptr<AboutPopup> popup = std::make_shared<AboutPopup>();
			return popup;
		}
	};

}
