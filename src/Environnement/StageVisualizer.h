#pragma once

namespace Environnement::StageVisualizer{

	void compile();
	void start();
	void stop();

	void canvas(glm::vec2 size_arg, float rounding = 0.0);
	void editor(glm::vec2 size_arg);

	void saveScript(const char* filePath);
	void loadScript(const char* filePath);
};
