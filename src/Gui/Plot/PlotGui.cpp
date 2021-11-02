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

void plotGui() {

	std::shared_ptr<Plot> currentPlot = Project::currentPlot;

	float sideBarWidth = ImGui::GetTextLineHeight() * 15.0;

	ImGui::BeginGroup();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
	ImGui::Button(currentPlot->name, glm::vec2(sideBarWidth, ImGui::GetTextLineHeight() * 1.5));
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();
	ImGui::PopFont();

	glm::vec2 listButtonSize((sideBarWidth - ImGui::GetStyle().ItemSpacing.x * 2.0) / 3.0, ImGui::GetTextLineHeight() * 2.0);

	float bottomSectionHeight = ImGui::GetTextLineHeightWithSpacing() + listButtonSize.y + ImGui::GetStyle().ItemSpacing.y * 1.0;

	std::vector<std::shared_ptr<Manoeuvre>>& manoeuvres = Project::currentPlot->manoeuvres;
	std::shared_ptr<Manoeuvre> clickedManoeuvre = nullptr;
	static DraggableList cueList;
	glm::vec2 cueListSize(sideBarWidth, ImGui::GetContentRegionAvail().y - bottomSectionHeight);
	if (cueList.beginList("##CueList", cueListSize, ImGui::GetTextLineHeight() * 0.3)) {
		float previousVerticalWindowPadding = ImGui::GetStyle().WindowPadding.y;
		//0 horizontal padding is to display the header background strip up to the edge of the cue window
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0, ImGui::GetTextLineHeight() * 0.2));
		glm::vec2 cueSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing() * 2.0 + ImGui::GetStyle().WindowPadding.y * 2.0);
		ImGuiWindowFlags cueWindowFlags = ImGuiWindowFlags_NoScrollbar;

		for (auto& manoeuvre : manoeuvres) {
			if (cueList.beginItem(cueSize, manoeuvre == currentPlot->selectedManoeuvre, cueWindowFlags)){
				manoeuvre->listGui();
				cueList.endItem();
			}
			if (ImGui::IsItemClicked()) clickedManoeuvre = manoeuvre;
		}
		ImGui::PopStyleVar();

		cueList.endList();
	}

	if (cueList.wasReordered()) {
		int oldIndex, newIndex;
		cueList.getReorderedItemIndex(oldIndex, newIndex);
		std::shared_ptr<Manoeuvre> tmp = manoeuvres[oldIndex];
		manoeuvres.erase(manoeuvres.begin() + oldIndex);
		manoeuvres.insert(manoeuvres.begin() + newIndex, tmp);
	}

	static const char* manoeuvreManagerString = "Manoeuvre Manager";
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::NewLine();
	ImGui::SameLine((sideBarWidth - ImGui::CalcTextSize(manoeuvreManagerString).x) / 2.0);
	ImGui::Text(manoeuvreManagerString);
	ImGui::PopFont();

	if (ImGui::Button("Create", listButtonSize)) currentPlot->addManoeuvre();
	ImGui::SameLine();
	bool disablSelectionBasedButtons = currentPlot->selectedManoeuvre == nullptr;
	if (disablSelectionBasedButtons) BEGIN_DISABLE_IMGUI_ELEMENT
	if (ImGui::Button("Duplicate", listButtonSize)) currentPlot->duplicateSelectedManoeuvre();
	ImGui::SameLine();
	if (ImGui::Button("Delete", listButtonSize)) currentPlot->deleteSelectedManoeuvre();
	if (disablSelectionBasedButtons) END_DISABLE_IMGUI_ELEMENT

	ImGui::EndGroup();

	ImGui::SameLine();

	if (ImGui::BeginChild("##CueProperties", ImGui::GetContentRegionAvail(), false)) {
	
		if (currentPlot->selectedManoeuvre == nullptr) {
			ImGui::PushFont(Fonts::robotoRegular20);
			ImGui::Text("No Manoeuvre Selected.");
			ImGui::PopFont();
		}
		else {
			std::shared_ptr<Manoeuvre> selectedManoeuvre = currentPlot->selectedManoeuvre;
			selectedManoeuvre->editGui();
		}
	}
	ImGui::EndChild();

	//we need to change selection after drawing the previously selected manoeuvre
	//else data might get wrongly copied
	if (clickedManoeuvre) currentPlot->selectManoeuvre(clickedManoeuvre);

}