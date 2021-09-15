#include <pch.h>

#include "Gui/Framework/GuiWindow.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/ioNodeFactory.h"
#include "Environnement/Environnement.h"

int main() {
	Logger::init();
	Timing::start();

	EtherCatDeviceFactory::loadDevices();
	ioNodeFactory::loadNodes();
	Environnement::load("xmlFile.xml");

	EtherCatFieldbus::updateNetworkInterfaceCardList();
	EtherCatFieldbus::init(EtherCatFieldbus::networkInterfaceCards.front());

	GuiWindow::open(3000,2000);
	GuiWindow::refresh();
	GuiWindow::close();

	EtherCatFieldbus::terminate();

	Logger::terminate();
}
