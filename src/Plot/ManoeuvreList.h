#pragma once

#include "Motion/MotionTypes.h"

namespace AnimationSystem{
	class Manoeuvre;
};
class Plot;
namespace tinyxml2{ struct XMLElement; }

class ManoeuvreList : public std::enable_shared_from_this<ManoeuvreList>{
public:
	
	ManoeuvreList(std::shared_ptr<Plot> plot_) : plot(plot_){}
	bool load(tinyxml2::XMLElement* xml);
	bool save(tinyxml2::XMLElement* xml);
	
	std::vector<std::shared_ptr<AnimationSystem::Manoeuvre>>& getManoeuvres(){ return manoeuvres; }
	bool containsManoeuvre(std::shared_ptr<AnimationSystem::Manoeuvre> manoeuvre);
	int getManoeuvreIndex(std::shared_ptr<AnimationSystem::Manoeuvre> manoeuvre);
	
	void selectNextManoeuvre(){}
	void selectPreviousManoeuvre(){}
	
	void addManoeuvre();
	void deleteSelectedManoeuvre(){}
	void duplicateSelectedManoeuvre(){}
	void moveManoeuvre(std::shared_ptr<AnimationSystem::Manoeuvre> m, int newIndex){}
	
	void pasteManoeuvre(std::shared_ptr<AnimationSystem::Manoeuvre> manoeuvre);

	//void refreshPlotAfterMachineLimitChanged(std::shared_ptr<Machine> m);
	//void refreshAll();
	
	std::shared_ptr<Plot> getPlot(){ return plot; }
	
private:
	
	std::shared_ptr<Plot> plot;
	std::vector<std::shared_ptr<AnimationSystem::Manoeuvre>> manoeuvres;
};
