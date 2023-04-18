#pragma once

#include "Legato/Gui/Window.h"

void etherCatGui();
void etherCatSlaves();
void etherCatMetrics();
void etherCatParameters();

class EtherCatStartPopup : public Legato::Popup{
public:
	EtherCatStartPopup() : Popup("Starting Environnement", true, false){}
	virtual void onDraw() override;
	virtual glm::vec2 getSize() override;
	SINGLETON_GET_METHOD(EtherCatStartPopup);
};

class EtherCatNetworkWindow : public Legato::Window{
public:
	EtherCatNetworkWindow() : Window("EtherCAT Network", true){}
	virtual void onDraw() override;
	SINGLETON_GET_METHOD(EtherCatNetworkWindow);
};

class EtherCatDevicesWindow : public Legato::Window{
public:
	EtherCatDevicesWindow() : Window("EtherCAT Devices", true){}
	virtual void onDraw() override;
	SINGLETON_GET_METHOD(EtherCatDevicesWindow);
};
