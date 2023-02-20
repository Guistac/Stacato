#pragma once

class Window;

namespace Stacato::Editor{

enum class Mode{
	LOCKED,
	PERFORMANCE,
	DESIGN,
	EXPERT
};

void initialize();
void terminate();

bool isLocked();
void unlock();
void lock();
bool checkEditorPassword(std::string& password);

std::vector<std::shared_ptr<Window>>& getUserWindows();
std::vector<std::shared_ptr<Window>>& getAdministratorWindows();

};
