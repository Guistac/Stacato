#pragma once

namespace Legato::Gui{

	void setInitializationFunction(std::function<void()> fn);
	void setGuiSubmitFunction(std::function<void()> fn);
	void setTerminationFunction(std::function<void()> fn);

	float getScale();

};

class Window;

namespace Legato::Gui::LayoutManager{

	void registerWindow(std::shared_ptr<Window> window);
	void unregisterWindow(std::shared_ptr<Window> window);

};
