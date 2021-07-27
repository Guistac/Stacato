#include "Fieldbus/EtherCatTest.h"
#include "Fieldbus/EthernetIpTest.h"
#include "Gui/GuiWindow.h"

int main() {
	GuiWindow::open();

	EthernetIPFieldbus::init("3.3.3.255");

	GuiWindow::refresh();


	GuiWindow::close();

	EthernetIPFieldbus::terminate();
}












