#include <pch.h>

#include "Project.h"

#include "Project/Environnement.h"
#include "Project/Plot.h"
#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Fieldbus/EtherCatFieldbus.h"

namespace Project {

	std::vector<std::shared_ptr<Plot>> plots;
	std::shared_ptr<Plot> currentPlot;
	char projectDirectory[512] = "";

	bool hasDefaultSavePath() {
		return strcmp(projectDirectory, "") != 0;
	}

	bool reload() {
		if(hasDefaultSavePath()) return load(projectDirectory);
		return false;
	}

	bool save() {
		if(hasDefaultSavePath()) return saveAs(projectDirectory);
		return false;
	}

	void createNew() {
		Environnement::createNew();
		plots.clear();
		currentPlot = nullptr;
		currentPlot = std::make_shared<Plot>();
		strcpy(currentPlot->name, "Default Plot");
		plots.push_back(currentPlot);
		strcpy(projectDirectory, "");
	}

	bool isEditingAllowed() {
		return !EtherCatFieldbus::isCyclicExchangeActive();
	}

};
