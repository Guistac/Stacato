#pragma once

namespace Environnement::StageVisualizer{

/*
	void initialize();
	void terminate();
*/
 
	void compile();
	void start();
	void stop();

	void canvas(glm::vec2 size_arg, float border = 0.0, float rounding = 0.0);
	void editor(glm::vec2 size_arg);

	void reset();
	void saveScript(const char* filePath);
	void loadScript(const char* filePath);
};
