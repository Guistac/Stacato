#pragma once

#include "Gui/ApplicationWindow/Window.h"

namespace Environnement{

namespace Script{

	bool canCompile();
	void compile();

	void start();
	void update();
	void stop();

	bool isRunning();

	void editor(glm::vec2 size_arg);

	void reset();
	void save(const char* filePath);
	void load(const char* filePath);

};
};
