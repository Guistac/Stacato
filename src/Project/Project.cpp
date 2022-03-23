#include <pch.h>

#include "Project.h"

#include "Project/Environnement.h"
#include "Project/Plot.h"

namespace Project {

	std::vector<std::shared_ptr<Plot>> plots;
	std::shared_ptr<Plot> currentPlot;



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
