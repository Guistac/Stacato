#pragma once

#include "Motion/MotionTypes.h"

class Manoeuvre;
class Plot;
namespace tinyxml2{ struct XMLElement; }

class ManoeuvreList : public std::enable_shared_from_this<ManoeuvreList>{
public:
	
	ManoeuvreList(std::shared_ptr<Plot> plot_) : plot(plot_){}
	bool load(tinyxml2::XMLElement* xml);
	bool save(tinyxml2::XMLElement* xml);
	
	std::vector<std::shared_ptr<Manoeuvre>>& getManoeuvres(){ return manoeuvres; }
	bool containsManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre);
	int getManoeuvreIndex(std::shared_ptr<Manoeuvre> manoeuvre);
	
	void selectNextManoeuvre(){}
	void selectPreviousManoeuvre(){}
	
	void addManoeuvre(/*ManoeuvreType type = ManoeuvreType::KEY*/){}
	void deleteSelectedManoeuvre(){}
	void duplicateSelectedManoeuvre(){}
	void moveManoeuvre(std::shared_ptr<Manoeuvre> m, int newIndex){}
	
	void pasteManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre);

	//void refreshPlotAfterMachineLimitChanged(std::shared_ptr<Machine> m);
	//void refreshAll();
	
	std::shared_ptr<Plot> getPlot(){ return plot; }
	
private:
	
	std::shared_ptr<Plot> plot;
	std::vector<std::shared_ptr<Manoeuvre>> manoeuvres;
};
