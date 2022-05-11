#include <pch.h>

#include "Plot.h"

#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Motion/Manoeuvre/ParameterTrack.h"

void Plot::selectManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre) {
	selectedManoeuvre = manoeuvre;
}

std::shared_ptr<Manoeuvre> Plot::getSelectedManoeuvre() {
	return selectedManoeuvre;
}

void Plot::addManoeuvre() {
	std::shared_ptr<Manoeuvre> newManoeuvre = std::make_shared<Manoeuvre>();
	static char newManoeuvreName[64];
	sprintf(newManoeuvreName, "M-%i", (int)manoeuvres.size());
	newManoeuvre->setName(newManoeuvreName);
	if (selectedManoeuvre == nullptr) manoeuvres.push_back(newManoeuvre);
	else {
		for (int i = 0; i < manoeuvres.size(); i++) {
			if (manoeuvres[i] == selectedManoeuvre) {
				manoeuvres.insert(manoeuvres.begin() + i + 1, newManoeuvre);
				break;
			}
		}
	}
}

void Plot::deleteSelectedManoeuvre() {
	if (selectedManoeuvre != nullptr) {
		for (int i = 0; i < manoeuvres.size(); i++) {
			if (manoeuvres[i] == selectedManoeuvre) {
				manoeuvres.erase(manoeuvres.begin() + i);
				break;
			}
		}
	}
	selectedManoeuvre = nullptr;
}

void Plot::duplicateSelectedManoeuvre() {
	if (selectedManoeuvre != nullptr) {
		std::shared_ptr<Manoeuvre> copiedManoeuvre = std::make_shared<Manoeuvre>(*selectedManoeuvre);
		for (int i = 0; i < manoeuvres.size(); i++) {
			if (manoeuvres[i] == selectedManoeuvre) {
				manoeuvres.insert(manoeuvres.begin() + i + 1, copiedManoeuvre);
				break;
			}
		}
		/*
		for (auto& track : copiedManoeuvre->tracks) {
			track->parentManoeuvre = copiedManoeuvre;
		}
		 */
	}
}

void Plot::reorderManoeuvre(std::shared_ptr<Manoeuvre> m, int oldIndex, int newIndex) {
	std::shared_ptr<Manoeuvre> tmp = m;
	manoeuvres.erase(manoeuvres.begin() + oldIndex);
	manoeuvres.insert(manoeuvres.begin() + newIndex, tmp);
}



void Plot::refreshPlotAfterMachineLimitChanged(std::shared_ptr<Machine> m) {
	//find all manoeuvres that have a parametertrack for the edited machine and revalidate the curves
}


void Plot::refreshAll() {
	/*
	refreshChainingDependencies();
	for (auto& manoeuvre : manoeuvres) {
		for (auto& track : manoeuvre->tracks) {
			track->refreshAfterChainedDependenciesRefresh();
		}
	}
	 */
}
