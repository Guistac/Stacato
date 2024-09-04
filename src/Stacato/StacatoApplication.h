#pragma once

namespace Stacato::Application{
	bool initialize(std::filesystem::path);
	bool terminate();
	bool requestQuit();
};
