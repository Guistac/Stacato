#include <pch.h>

#include "Gui/GuiWindow.h"
#include "Fieldbus/EtherCatFieldbus.h"

int main() {
	Logger::init();

	EtherCatFieldbus::updateNetworkInterfaceCardList();
	EtherCatFieldbus::init(EtherCatFieldbus::networkInterfaceCards.front());

	GuiWindow::open(1920,1080);
	GuiWindow::refresh();
	GuiWindow::close();

	EtherCatFieldbus::terminate();

	Logger::terminate();
}
