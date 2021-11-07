#pragma once

class Manoeuvre;

class Plot {
public:

	Plot() {}

	char name[256] = "";
	std::time_t saveTime;

	std::vector<std::shared_ptr<Manoeuvre>> manoeuvres;
	std::shared_ptr<Manoeuvre> selectedManoeuvre = nullptr;

	bool save(const char* path);
	bool load(const char* path);

	void selectManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre);
	std::shared_ptr<Manoeuvre> getSelectedManoeuvre();
	void addManoeuvre();
	void deleteSelectedManoeuvre();
	void duplicateSelectedManoeuvre();

	void refresh();
 
};

