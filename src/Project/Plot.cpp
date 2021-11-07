#include <pch.h>

#include "Plot.h"

#include "Motion/Manoeuvre/Manoeuvre.h"

void Plot::selectManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre) {
	selectedManoeuvre = manoeuvre;
}

std::shared_ptr<Manoeuvre> Plot::getSelectedManoeuvre() {
	return selectedManoeuvre;
}

void Plot::addManoeuvre() {
	std::shared_ptr<Manoeuvre> newManoeuvre = std::make_shared<Manoeuvre>();
	sprintf(newManoeuvre->name, "M-%i", (int)manoeuvres.size());
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
		sprintf(copiedManoeuvre->name, "%s*", selectedManoeuvre->name);
		sprintf(copiedManoeuvre->description, "(copy) %s", selectedManoeuvre->description);
		for (int i = 0; i < manoeuvres.size(); i++) {
			if (manoeuvres[i] == selectedManoeuvre) {
				manoeuvres.insert(manoeuvres.begin() + i + 1, copiedManoeuvre);
				break;
			}
		}
	}
}


void Plot::refresh() {
	
}
