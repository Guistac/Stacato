#pragma once

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
	void save(std::string folderPath);
	void load(std::string folderPath);

};
};
