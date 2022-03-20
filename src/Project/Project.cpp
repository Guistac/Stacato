#include <pch.h>

#include "Project.h"

#include "Project/Environnement.h"
#include "Project/Plot.h"
#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include "Gui/ApplicationWindow/ApplicationWindow.h"

namespace Project {

	std::vector<std::shared_ptr<Plot>> plots;
	std::shared_ptr<Plot> currentPlot;

	bool b_plotEditingAllowed = false;

	void createNew() {
		Environnement::createNew();
		plots.clear();
		currentPlot = nullptr;
		currentPlot = std::make_shared<Plot>();
		strcpy(currentPlot->name, "Default Plot");
		plots.push_back(currentPlot);
		saveFilePath[0] = 0;
		b_hasFilePath = false;
		ApplicationWindow::setWindowName("New Project");
	}

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
