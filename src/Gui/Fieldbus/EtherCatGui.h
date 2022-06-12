#pragma once

#include "Gui/ApplicationWindow/Window.h"

void etherCatGui();
void etherCatSlaves();
void etherCatMetrics();
void etherCatParameters(bool resetList);

class EtherCatStartPopup : public Popup{
public:
	
	EtherCatStartPopup() : Popup("Starting Environnement", true, false){}
	
	virtual void drawContent() override;
	virtual glm::vec2 getSize() override;
	
	static std::shared_ptr<EtherCatStartPopup> get(){
		static auto popup = std::make_shared<EtherCatStartPopup>();
		return popup;
	}
	
};
