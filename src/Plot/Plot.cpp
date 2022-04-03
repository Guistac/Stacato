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
	std::shared_ptr<Manoeuvre> newManoeuvre = std::make_shared<Manoeuvre>(shared_from_this());
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
	refreshChainingDependencies();
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
	refreshChainingDependencies();
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
		for (auto& track : copiedManoeuvre->tracks) {
			track->parentManoeuvre = copiedManoeuvre;
		}
	}
	refreshChainingDependencies();
}

void Plot::reorderManoeuvre(std::shared_ptr<Manoeuvre> m, int oldIndex, int newIndex) {
	std::shared_ptr<Manoeuvre> tmp = m;
	manoeuvres.erase(manoeuvres.begin() + oldIndex);
	manoeuvres.insert(manoeuvres.begin() + newIndex, tmp);
	refreshChainingDependencies();
}


void Plot::refreshChainingDependencies() {

	for (auto& manoeuvre : manoeuvres) {
		for (auto& track : manoeuvre->tracks) {
			track->nextChainedMasterTrack = nullptr;
			track->nextChainedSlaveTrack = nullptr;
			track->previousChainedMasterTrack = nullptr;
			track->previousChainedSlaveTrack = nullptr;
		}
	}

	for (int i = 0; i < manoeuvres.size(); i++) {
		int currentManoeuvreIndex = i;
		std::shared_ptr<Manoeuvre> currentManoeuvre = manoeuvres[currentManoeuvreIndex];

		for (auto& track : currentManoeuvre->tracks) {
			
			bool trackNeedsRefresh = false;

			if (track->originIsPreviousTarget) {
				bool foundChainedTrack = false;
				for (int j = currentManoeuvreIndex - 1; j >= 0; j--) {
					if (manoeuvres[j]->hasTrack(track->parameter)) {
						std::shared_ptr<ParameterTrack> previousChainedMasterTrack = manoeuvres[j]->getTrack(track->parameter);
						if (!track->origin.equals(previousChainedMasterTrack->target)) {
							track->origin = previousChainedMasterTrack->target;
							trackNeedsRefresh = true;
						}
						track->previousChainedMasterTrack = previousChainedMasterTrack;
						previousChainedMasterTrack->nextChainedSlaveTrack = track;
						foundChainedTrack = true;
						break;
					}
				}
				if (!foundChainedTrack) {
					track->previousChainedMasterTrack = nullptr;
					trackNeedsRefresh = true;
				}
			}

			if (track->targetIsNextOrigin) {
				bool foundChainedTrack = false;
				for (int j = currentManoeuvreIndex + 1; j < manoeuvres.size(); j++) {
					if (manoeuvres[j]->hasTrack(track->parameter)) {
						std::shared_ptr<ParameterTrack> nextChainedMasterTrack = manoeuvres[j]->getTrack(track->parameter);
						if (!track->target.equals(nextChainedMasterTrack->origin)) {
							track->target = nextChainedMasterTrack->origin;
							trackNeedsRefresh = true;
						}
						track->nextChainedMasterTrack = nextChainedMasterTrack;
						nextChainedMasterTrack->previousChainedSlaveTrack = track;
						foundChainedTrack = true;
						break;
					}
				}
				if (!foundChainedTrack) {
					track->nextChainedMasterTrack = nullptr;
					trackNeedsRefresh = true;
				}
			}

			if(trackNeedsRefresh) track->refreshAfterChainedDependenciesRefresh();

		}
		currentManoeuvre->refresh();
	}
}



void Plot::refreshPlotAfterMachineLimitChanged(std::shared_ptr<Machine> m) {
	//find all manoeuvres that have a parametertrack for the edited machine and revalidate the curves
}


void Plot::refreshAll() {
	refreshChainingDependencies();
	for (auto& manoeuvre : manoeuvres) {
		for (auto& track : manoeuvre->tracks) {
			track->refreshAfterChainedDependenciesRefresh();
		}
	}
}
