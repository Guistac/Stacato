#include <pch.h>

#include "Project.h"

#include "Environnement/Environnement.h"
#include "Plot/Plot.h"

namespace Project {

	std::vector<std::shared_ptr<Plot>> plots;
	std::shared_ptr<Plot> currentPlot;

	std::vector<std::shared_ptr<Plot>>& getPlots(){ return plots; }

	std::shared_ptr<Plot> getCurrentPlot(){ return currentPlot; }

	void setCurrentPlot(std::shared_ptr<Plot> plot){
		if(plot == nullptr) return;
		currentPlot = plot;
	}

	std::shared_ptr<Plot> createNewPlot(){
		auto newPlot = Plot::create();
		plots.push_back(newPlot);
		currentPlot = newPlot;
		return newPlot;
	}

	void deletePlot(std::shared_ptr<Plot> plot){
		if(plots.size() <= 1) return;
		for(int i = 0; i < plots.size(); i++){
			if(plots[i] == plot){
				plots.erase(plots.begin() + i);
				break;
			}
		}
		if(plot == currentPlot) currentPlot = plots.front();
	}

	void deleteAllPlots(){
		plots.clear();
		currentPlot = nullptr;
	}

	void addPlot(std::shared_ptr<Plot> plot){
		plots.push_back(plot);
	}

	void duplicatePlot(std::shared_ptr<Plot> plot){
		auto copy = plot->duplicate();
		plots.push_back(copy);
		currentPlot = copy;
	}





	bool canCloseImmediately(){
		return !hasUnsavedModifications() && !Environnement::isRunning();
	}



	bool b_plotEditingAllowed = false;

	bool isEditingAllowed() {
		return !Environnement::isRunning();
	}

	void setPlotEdit(bool isAllowed){
		b_plotEditingAllowed = isAllowed;
	}

	bool isPlotEditAllowed(){
		return b_plotEditingAllowed;
	}
	
};
