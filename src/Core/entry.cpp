#include <pch.h>

#include <Gui/GuiWindow.h>
#include <Networking/Network.h>
#include <Core/timing.h>

int main() {
	Timing::start();

	//Initialisation
	Network::init();
	GuiWindow::open();

	//Gui Runtime
	GuiWindow::refresh();

	//Termination
	GuiWindow::close();
	Network::terminate();
}