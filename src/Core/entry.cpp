#include <pch.h>

#include "Gui/Framework/GuiWindow.h"
#include "Fieldbus/EtherCatFieldbus.h"

int main() {
	Logger::init();
	Timing::start();

	EtherCatFieldbus::updateNetworkInterfaceCardList();
	EtherCatFieldbus::init(EtherCatFieldbus::networkInterfaceCards.front());

	GuiWindow::open(1920,1920);
	GuiWindow::refresh();
	GuiWindow::close();

	EtherCatFieldbus::terminate();

	Logger::terminate();
}
