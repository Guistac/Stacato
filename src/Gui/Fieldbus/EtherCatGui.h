#pragma once

#include "Gui/ApplicationWindow/Window.h"

void etherCatGui();
void etherCatSlaves();
void etherCatMetrics();
void etherCatParameters();

class EtherCatStartPopup : public Popup{
public:
	EtherCatStartPopup() : Popup("Starting Environnement", true, false){}
	virtual void drawContent() override;
	virtual glm::vec2 getSize() override;
	SINGLETON_GET_METHOD(EtherCatStartPopup);
};

class EtherCatNetworkWindow : public Window{
public:
	EtherCatNetworkWindow() : Window("EtherCAT Network", true){}
	virtual void drawContent() override;
	SINGLETON_GET_METHOD(EtherCatNetworkWindow);
};

class EtherCatDevicesWindow : public Window{
public:
	EtherCatDevicesWindow() : Window("EtherCAT Devices", true){}
	virtual void drawContent() override;
	SINGLETON_GET_METHOD(EtherCatDevicesWindow);
};
