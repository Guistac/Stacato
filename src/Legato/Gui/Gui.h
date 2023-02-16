#pragma once

namespace NewGui{

	void setInitializationFunction(std::function<void()> fn);
	void setGuiSubmitFunction(std::function<void()> fn);
	void setTerminationFunction(std::function<void()> fn);

	class Window;

	void open(std::shared_ptr<Window> window);
	void close(std::shared_ptr<Window> window);

	float getScale();

	void drawWindows();

};
