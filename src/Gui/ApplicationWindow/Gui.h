#pragma once

#include "Window.h"

namespace Gui{

	void initialize();
	void draw();
	
	void menuBar();
	void toolbar(float height);

	void setDefaultLayout();

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
