#pragma once

#include "Legato/Gui/Window.h"

namespace Stacato::Gui{

	void initialize();
	void terminate();
	void gui();

	void newProject();
	void load();
	bool save();
	bool saveAs();

	class AboutPopup : public Legato::Popup{
	public:
		AboutPopup() : Popup("About", true, true){}
		SINGLETON_GET_METHOD(AboutPopup);
		virtual void onDraw() override;
	};

	class QuitApplicationPopup : public Legato::Popup{
	public:
		QuitApplicationPopup() : Popup("Quit Application", true, true){}
		SINGLETON_GET_METHOD(QuitApplicationPopup);
		virtual void onDraw() override;
	};

	class CloseProjectPopup : public Legato::Popup{
	public:
		CloseProjectPopup() : Popup("Close Project", true, true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(CloseProjectPopup);
	};

};
