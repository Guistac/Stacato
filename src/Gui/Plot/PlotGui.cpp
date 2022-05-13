#include <pch.h>

#include <imgui.h>

#include "Gui/Utilities/DraggableList.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Project/Project.h"
#include "Plot/Plot.h"
#include "Motion/Manoeuvre/Manoeuvre.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Motion/Playback/Playback.h"


#include "Gui/Utilities/DraggableListNew.h"

namespace PlotGui{

	void manoeuvreList() {

		glm::vec2 managerButtonSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0) / 3.0, ImGui::GetTextLineHeight() * 2.0);
		float managerHeight = managerButtonSize.y + ImGui::GetStyle().ItemSpacing.y;
		glm::vec2 manoeuvreListSize = ImGui::GetContentRegionAvail();
		manoeuvreListSize.y -= managerHeight;
		
		//================= MANOEUVRE LIST =======================

		static DraggableList manoeuvreList;
		std::shared_ptr<Plot> currentPlot = Project::currentPlot;
		std::vector<std::shared_ptr<Manoeuvre>>& manoeuvres = Project::currentPlot->getManoeuvres();
		std::shared_ptr<Manoeuvre> clickedManoeuvre = nullptr;
		
		if (manoeuvreList.beginList("##CueList", manoeuvreListSize, ImGui::GetTextLineHeight() * 0.3)) {
			float previousVerticalWindowPadding = ImGui::GetStyle().WindowPadding.y;
			//0 horizontal padding is to display the header background strip up to the edge of the cue window
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0, ImGui::GetTextLineHeight() * 0.2));
			glm::vec2 cueSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing() * 2.0 + ImGui::GetStyle().WindowPadding.y * 2.0);
			ImGuiWindowFlags cueWindowFlags = ImGuiWindowFlags_NoScrollbar;

			for (auto& manoeuvre : manoeuvres) {
				if (manoeuvreList.beginItem(cueSize, manoeuvre == currentPlot->getSelectedManoeuvre(), cueWindowFlags)){
					manoeuvre->listGui();
					manoeuvreList.endItem();
				}
				if (ImGui::IsItemClicked()) clickedManoeuvre = manoeuvre;
			}
			ImGui::PopStyleVar();

			manoeuvreList.endList();
		}
		
		//list interaction
		if (clickedManoeuvre) currentPlot->selectManoeuvre(clickedManoeuvre);
		if (manoeuvreList.wasReordered()) {
			int oldIndex, newIndex;
			manoeuvreList.getReorderedItemIndex(oldIndex, newIndex);
			currentPlot->reorderManoeuvre(manoeuvres[oldIndex], oldIndex, newIndex);
		}

		//================== MANOEUVER MANAGER BUTTONS ========================

		if (ImGui::Button("Create", managerButtonSize)) currentPlot->addManoeuvre();
		ImGui::SameLine();
		ImGui::BeginDisabled(currentPlot->getSelectedManoeuvre() == nullptr);
		if (ImGui::Button("Duplicate", managerButtonSize)) currentPlot->duplicateSelectedManoeuvre();
		ImGui::SameLine();
		if (ImGui::Button("Delete", managerButtonSize)) currentPlot->deleteSelectedManoeuvre();
		ImGui::EndDisabled();
		
	}


bool noSelectionDisplay(){
	if (Project::currentPlot->getSelectedManoeuvre() == nullptr) {
		ImGui::PushFont(Fonts::sansRegular20);
		ImGui::Text("%s", "No Manoeuvre Selected");
		ImGui::PopFont();
		return true;
	}
	
}

void trackSheetEditor(){
	if(noSelectionDisplay()) return;
	Project::currentPlot->getSelectedManoeuvre()->trackSheetGui();
}

void curveEditor(){
	if(noSelectionDisplay()) return;
	Project::currentPlot->getSelectedManoeuvre()->curveEditorGui();
}

void spatialEditor(){
	if(noSelectionDisplay()) return;
	Project::currentPlot->getSelectedManoeuvre()->spatialEditorGui();
}


void selectedManoeuvrePlaybackControl(float height){
	
	UpDownButtons::Interaction interaction = UpDownButtons::draw("", height / 2.0);
	if(interaction == UpDownButtons::Interaction::UP) Project::currentPlot->selectPreviousManoeuvre();
	else if(interaction == UpDownButtons::Interaction::DOWN) Project::currentPlot->selectNextManoeuvre();
	
	ImGui::SameLine();
	
	glm::vec2 manoeuvreDisplaySize(ImGui::GetTextLineHeight() * 5.0, height);
	
	auto selectedManoeuvre = Project::currentPlot->getSelectedManoeuvre();
	bool b_noSelection = selectedManoeuvre == nullptr;
	if(b_noSelection) {
		ImGui::PushFont(Fonts::sansRegular15);
		backgroundText("No Selection", manoeuvreDisplaySize, glm::vec4(0.2, 0.2, 0.2, 1.0));
		ImGui::PopFont();
	}
	else selectedManoeuvre->miniatureGui(manoeuvreDisplaySize);
	
	ImGui::SameLine();
	
	bool b_disableRapidToStart = b_noSelection || selectedManoeuvre->getType() != ManoeuvreType::SEQUENCE;
	bool b_disableRapidToEnd = b_noSelection;
	
	switch(NextPreviousButtons::draw("rapidToStartEnd", height, true, b_disableRapidToStart, b_disableRapidToEnd)){
		case NextPreviousButtons::Interaction::NONE: break;
		case NextPreviousButtons::Interaction::PREVIOUS: break;
		case NextPreviousButtons::Interaction::NEXT: break;
	}
	
	ImGui::SameLine();
	
	static double time_seconds_p = 12345.0;
	static double time_seconds_n = -2345.0;
	ImGui::BeginGroup();
	ImGui::PushFont(Fonts::sansBold12);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(2));
	timeEntryWidgetSeconds("##timeFromStart", (height / 2.0) - 1, time_seconds_p);
	timeEntryWidgetSeconds("##timeToEnd", (height / 2.0) - 1, time_seconds_n);
	ImGui::PopStyleVar();
	ImGui::PopFont();
	ImGui::EndGroup();
	
	ImGui::SameLine();
	ImGui::BeginDisabled(b_noSelection || selectedManoeuvre->getType() != ManoeuvreType::SEQUENCE);
	buttonArrowDownStop("rapidToPlaybackPosition", height);
	ImGui::EndDisabled();
	
	ImGui::SameLine();
	ImGui::BeginDisabled(b_noSelection || selectedManoeuvre->getType() == ManoeuvreType::KEY);
	buttonPlay("PlayManoeuvre", height); //or pause button if we are playing
	ImGui::EndDisabled();
	
	ImGui::SameLine();
	ImGui::BeginDisabled(b_noSelection);
	buttonStop("Stop", height);
	ImGui::EndDisabled();
	
	ImGui::SameLine();
	buttonSTOP("StopAll", height);
	
}

}
