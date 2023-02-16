#pragma once

#include "Legato/Gui/Window.h"

namespace Project::Gui{

	//FileIO
	void load();
	bool save();
	bool saveAs();

	class CloseProjectPopup : public Popup{
	public:
		CloseProjectPopup() : Popup("Close Project", true, true){}
		virtual void onDraw() override;
		SINGLETON_GET_METHOD(CloseProjectPopup);
	};
	

}
