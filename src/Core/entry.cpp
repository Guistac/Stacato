#include <thread>
#include <iostream>
#include "Gui/GuiWindow.h"

int main() {
	unsigned int cpuCoreCount = std::thread::hardware_concurrency();
	std::cout << cpuCoreCount << std::endl;

	GuiWindow::open();
	GuiWindow::refresh();
	GuiWindow::close();
}












