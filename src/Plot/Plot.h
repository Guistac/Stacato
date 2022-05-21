#pragma once

class Manoeuvre;
class ParameterTrack;
class Machine;

class Plot : public std::enable_shared_from_this<Plot>{

	//—————————————————————————————————————
	//			Saving & Loading
	//—————————————————————————————————————

public:
	
	static std::shared_ptr<Plot> load(std::string& filePath);
	bool save(std::string& filePath);

	//—————————————————————————————————————
	//		   General Properties
	//—————————————————————————————————————
	
public:
	
	const char* getName(){ return name.c_str(); }
	void setName(std::string name_){ name = name_; }
	
private:
	
	std::string name;
	
	//—————————————————————————————————————
	//			Manoeuvre List
	//—————————————————————————————————————
	
public:
	
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
	
private:
	
	std::vector<std::shared_ptr<Manoeuvre>> manoeuvres;
	std::shared_ptr<Manoeuvre> selectedManoeuvre = nullptr;
	
	//—————————————————————————————————————
	//				Others..
	//—————————————————————————————————————
	
private:
	
	std::time_t saveTime;
	
};

