#pragma once

#include "Legato/Gui/Window.h"

namespace Stacato::Gui{

	void initialize();
	void terminate();
	void gui();

	std::vector<std::shared_ptr<Window>>& getUserWindows();
	std::vector<std::shared_ptr<Window>>& getAdministratorWindows();

	void newProject();
	void load();
	bool save();
	bool saveAs();

	class AboutPopup : public Popup{
	public:
		AboutPopup() : Popup("About", true, true){}
		SINGLETON_GET_METHOD(AboutPopup);
		virtual void onDraw() override;
	};

	class QuitApplicationPopup : public Popup{
	public:
		QuitApplicationPopup() : Popup("Quit Application", true, true){}
		SINGLETON_GET_METHOD(QuitApplicationPopup);
		virtual void onDraw() override;
	};

	class CloseProjectPopup : public Popup{
	public:
		CloseProjectPopup() : Popup("Close Project", true, true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(CloseProjectPopup);
	};

};
