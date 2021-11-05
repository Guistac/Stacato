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

	float sideBarWidth = ImGui::GetTextLineHeight() * 15.0;
	ImGui::BeginChild("##ManoeuvreList", glm::vec2(sideBarWidth, ImGui::GetContentRegionAvail().y), false);

	std::shared_ptr<Plot> currentPlot = Project::currentPlot;
	std::shared_ptr<Manoeuvre> selectedManoeuvre = currentPlot->getSelectedManoeuvre();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
	ImGui::Button(currentPlot->name, glm::vec2(sideBarWidth, ImGui::GetTextLineHeight() * 1.5));
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();
	ImGui::PopFont();

	glm::vec2 managerButtonSize((sideBarWidth - ImGui::GetStyle().ItemSpacing.x * 2.0) / 3.0, ImGui::GetTextLineHeight() * 2.0);
	float bottomSectionHeight = ImGui::GetTextLineHeightWithSpacing() * 2.0;	//two title text lines
	bottomSectionHeight += managerButtonSize.y * 2.0;							//two button rows (manager buttons & stop buttons)
	bottomSectionHeight += ImGui::GetStyle().ItemSpacing.y * 2.0;				//spacing for the two button rows
	if (selectedManoeuvre) bottomSectionHeight += selectedManoeuvre->getPlaybackControlGuiHeight();
	else bottomSectionHeight += ImGui::GetTextLineHeightWithSpacing();			//if no selection, single text line for no selection display

	//================= MANOEUVRE LIST =======================

	std::vector<std::shared_ptr<Manoeuvre>>& manoeuvres = Project::currentPlot->manoeuvres;
	std::shared_ptr<Manoeuvre> clickedManoeuvre = nullptr;
	static DraggableList manoeuvreList;
	glm::vec2 manoeuvreListSize(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - bottomSectionHeight);
	if (manoeuvreList.beginList("##CueList", manoeuvreListSize, ImGui::GetTextLineHeight() * 0.3)) {
		float previousVerticalWindowPadding = ImGui::GetStyle().WindowPadding.y;
		//0 horizontal padding is to display the header background strip up to the edge of the cue window
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0, ImGui::GetTextLineHeight() * 0.2));
		glm::vec2 cueSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing() * 2.0 + ImGui::GetStyle().WindowPadding.y * 2.0);
		ImGuiWindowFlags cueWindowFlags = ImGuiWindowFlags_NoScrollbar;

		for (auto& manoeuvre : manoeuvres) {
			if (manoeuvreList.beginItem(cueSize, manoeuvre == currentPlot->selectedManoeuvre, cueWindowFlags)){
				manoeuvre->listGui();
				manoeuvreList.endItem();
			}
			if (ImGui::IsItemClicked()) clickedManoeuvre = manoeuvre;
		}
		ImGui::PopStyleVar();

		manoeuvreList.endList();
	}
	if (manoeuvreList.wasReordered()) {
		int oldIndex, newIndex;
		manoeuvreList.getReorderedItemIndex(oldIndex, newIndex);
		std::shared_ptr<Manoeuvre> tmp = manoeuvres[oldIndex];
		manoeuvres.erase(manoeuvres.begin() + oldIndex);
		manoeuvres.insert(manoeuvres.begin() + newIndex, tmp);
	}


	//================== MANOEUVER MANAGER BUTTONS ========================

	static const char* manoeuvreManagerString = "Manoeuvre Manager";
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::NewLine();
	ImGui::SameLine((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(manoeuvreManagerString).x) / 2.0);
	ImGui::Text(manoeuvreManagerString);
	ImGui::PopFont();

	if (ImGui::Button("Create", managerButtonSize)) currentPlot->addManoeuvre();
	ImGui::SameLine();
	bool disablSelectionBasedButtons = selectedManoeuvre == nullptr;
	if (disablSelectionBasedButtons) BEGIN_DISABLE_IMGUI_ELEMENT
	if (ImGui::Button("Duplicate", managerButtonSize)) currentPlot->duplicateSelectedManoeuvre();
	ImGui::SameLine();
	if (ImGui::Button("Delete", managerButtonSize)) currentPlot->deleteSelectedManoeuvre();
	if (disablSelectionBasedButtons) END_DISABLE_IMGUI_ELEMENT

	static const char* playbackControlsString = "Playback Controls";
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::NewLine();
	ImGui::SameLine((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(playbackControlsString).x) / 2.0);
	ImGui::Text(playbackControlsString);
	ImGui::PopFont();

	//===================== PLAYBACK MANAGER BUTTONS =======================

	static const char* noManoeuvreSelectedString = "No Manoeuvre Selected";

	if (selectedManoeuvre) {
		selectedManoeuvre->playbackControlGui();
	}
	else {
		ImGui::NewLine();
		ImGui::SameLine((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(noManoeuvreSelectedString).x) / 2.0);
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("No Manoeuvre Selected");
		ImGui::PopStyleColor();
	}

	if (ImGui::Button("Stop All", glm::vec2(ImGui::GetContentRegionAvail().x, managerButtonSize.y))) currentPlot->stopAllManoeuvres();

	ImGui::EndChild();

	ImGui::SameLine();

	if (ImGui::BeginChild("##CueProperties", ImGui::GetContentRegionAvail(), false)) {
	
		if (currentPlot->selectedManoeuvre == nullptr) {
			ImGui::PushFont(Fonts::robotoRegular20);
			ImGui::Text(noManoeuvreSelectedString);
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