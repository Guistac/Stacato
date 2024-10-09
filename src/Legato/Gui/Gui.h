#pragma once

namespace Legato::Gui{

	void setInitializationFunction(std::function<void()> fn);
	void setGuiSubmitFunction(std::function<void()> fn);
	void setTerminationFunction(std::function<void()> fn);

	float getScale();

	void makeMainWindowFullscreen();

};

class Window;
class Layout;

namespace Legato::Gui::WindowManager{

	bool areWindowsLocked();
	bool unlockWindows();
	bool lockWindows();

	void registerWindow(std::shared_ptr<Window> window);
	void unregisterWindow(std::shared_ptr<Window> window);

	std::shared_ptr<Layout> captureCurentLayout();
	void applyLayout(std::shared_ptr<Layout> layout);

};
