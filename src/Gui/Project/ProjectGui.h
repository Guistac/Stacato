#pragma once

#include "Gui/ApplicationWindow/Window.h"

namespace Project::Gui{

	//FileIO
	void load();
	bool save();
	bool saveAs();

	class CloseProjectPopup : public Popup{
	public:
		CloseProjectPopup() : Popup("Close Project", true, true){}
		virtual void drawContent() override;
		static std::shared_ptr<CloseProjectPopup> get(){
			static auto popup = std::make_shared<CloseProjectPopup>();
			return popup;
		}
	};
	

}
