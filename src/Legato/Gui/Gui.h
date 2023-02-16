#pragma once

namespace Legato::Gui{

	void setInitializationFunction(std::function<void()> fn);
	void setGuiSubmitFunction(std::function<void()> fn);
	void setTerminationFunction(std::function<void()> fn);

	float getScale();
};
