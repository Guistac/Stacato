#include <pch.h>

#include "PlotGui.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Utilities/DraggableList.h"
#include "Project/Project.h"
#include "Plot/Plot.h"
#include "Plot/Manoeuvre.h"
#include "Gui/Framework/Fonts.h"
#include "Gui/Framework/Colors.h"

void manoeuvreListGui() {
	
	static DraggableList cueList;

	std::shared_ptr<Plot> currentPlot = Project::currentPlot;
	std::vector<std::shared_ptr<Manoeuvre>>& manoeuvres = Project::currentPlot->manoeuvres;

	glm::vec2 cueListSize(ImGui::GetTextLineHeight() * 15.0, ImGui::GetContentRegionAvail().y);

	if (cueList.beginList("##CueList", cueListSize, ImGui::GetTextLineHeight() * 0.3)) {
	
		glm::vec2 cueSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 3.0);

		for (auto& manoeuvre : manoeuvres) {
			if (cueList.beginItem(cueSize, manoeuvre == currentPlot->selectedManoeuvre)){
				manoeuvre->listGui();
				cueList.endItem();
			}
			if (ImGui::IsItemClicked()) {
				currentPlot->selectedManoeuvre = manoeuvre;
			}
		}

		cueList.endList();

		if (cueList.wasReordered()) {
			int oldIndex, newIndex;
			cueList.getReorderedItemIndex(oldIndex, newIndex);
			std::shared_ptr<Manoeuvre> tmp = manoeuvres[oldIndex];
			manoeuvres.erase(manoeuvres.begin() + oldIndex);
			manoeuvres.insert(manoeuvres.begin() + newIndex, tmp);
		}
	}

	ImGui::SameLine();

	if (ImGui::BeginChild("##CueProperties", ImGui::GetContentRegionAvail(), false)) {
	
		if (currentPlot->selectedManoeuvre == nullptr) {
			ImGui::PushFont(Fonts::robotoRegular42);
			ImGui::Text("No Manoeuvre Selected.");
			ImGui::PopFont();
		}
		else {
			std::shared_ptr<Manoeuvre> selectedManoeuvre = currentPlot->selectedManoeuvre;
			selectedManoeuvre->editGui();
		}
	}
	ImGui::EndChild();





}