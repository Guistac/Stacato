#pragma once

class Console;
class ConsoleMapping;

namespace ConsoleHandler{

	void initialize();
	void terminate();

	void applyMapping(std::shared_ptr<Console> console);

	std::shared_ptr<Console> getConnectedConsole();

	void gui(float height);
	
};
