#pragma once

class ManoeuvreList;
class Manoeuvre;

class Plot : public std::enable_shared_from_this<Plot>{

	//—————————————————————————————————————
	//			Saving & Loading
	//—————————————————————————————————————

public:
	
	
	static std::shared_ptr<Plot> create();
	static std::shared_ptr<Plot> load(std::string& filePath);
	std::shared_ptr<Plot> duplicate();
	bool save(std::string& filePath);
	bool isCurrent();
	
	//—————————————————————————————————————
	//		   General Properties
	//—————————————————————————————————————
	
public:
	
	const char* getName(){ return name.c_str(); }
	void setName(std::string name_){ name = name_; }
	
	std::shared_ptr<ManoeuvreList> getManoeuvreList(){ return manoeuvreList; }
	std::shared_ptr<Manoeuvre> getSelectedManoeuvre(){ return selectedManoeuvre; }
	void selectManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre);
	
	bool b_scrollToSelectedManoeuvre = false;
	
private:
	
	std::string name;
	std::shared_ptr<ManoeuvreList> manoeuvreList;
	std::shared_ptr<Manoeuvre> selectedManoeuvre;
	
	//—————————————————————————————————————
	//				Others..
	//—————————————————————————————————————
	
private:
	
	std::time_t saveTime;
	
};
