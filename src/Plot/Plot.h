#pragma once

class Manoeuvre;
class ParameterTrack;
class Machine;

class Plot : public std::enable_shared_from_this<Plot>{
public:

	Plot() {}

	const char* getName(){ return name; }
	void setName(const char* name_){ strcpy(name, name_); }
	
	std::vector<std::shared_ptr<Manoeuvre>>& getManoeuvres(){ return manoeuvres; }
	
	void selectManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre);
	std::shared_ptr<Manoeuvre> getSelectedManoeuvre();
	
	int getManoeuvreIndex(std::shared_ptr<Manoeuvre> manoeuvre);
	void selectNextManoeuvre();
	void selectPreviousManoeuvre();
	
	void addManoeuvre();
	void deleteSelectedManoeuvre();
	void duplicateSelectedManoeuvre();
	void reorderManoeuvre(std::shared_ptr<Manoeuvre> m, int newIndex);

	void refreshPlotAfterMachineLimitChanged(std::shared_ptr<Machine> m);
	void refreshAll();
	
	bool save(const char* path);
	bool load(const char* path);
	
private:

	std::vector<std::shared_ptr<Manoeuvre>> manoeuvres;
	std::shared_ptr<Manoeuvre> selectedManoeuvre = nullptr;

	char name[256] = "";
	std::time_t saveTime;
	
};

