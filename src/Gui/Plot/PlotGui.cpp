#include <pch.h>

#include <imgui.h>

#include "Gui/Utilities/ReorderableList.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Project/Project.h"
#include "Plot/Plot.h"
#include "Animation/Manoeuvre.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Animation/Playback/Playback.h"

#include "Plot/ManoeuvreList.h"

#include "Gui/Utilities/ReorderableList.h"

namespace PlotGui{

	void manoeuvreList() {

		glm::vec2 managerButtonSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0) / 3.0, ImGui::GetTextLineHeight() * 2.0);
		float managerHeight = managerButtonSize.y + ImGui::GetStyle().ItemSpacing.y;
		glm::vec2 manoeuvreListSize = ImGui::GetContentRegionAvail();
		manoeuvreListSize.y -= managerHeight;
		
		//================= MANOEUVRE LIST =======================

		
		std::shared_ptr<Plot> plot = Project::currentPlot;
		std::shared_ptr<ManoeuvreList> manoeuvreList = plot->getManoeuvreList();
		std::vector<std::shared_ptr<Manoeuvre>>& manoeuvres = manoeuvreList->getManoeuvres();
		std::shared_ptr<Manoeuvre> clickedManoeuvre = nullptr;
		
		if(ReorderableList::begin("CueList", manoeuvreListSize)){
		
			float previousVerticalWindowPadding = ImGui::GetStyle().WindowPadding.y;
			//0 horizontal padding is to display the header background strip up to the edge of the cue window
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0, ImGui::GetTextLineHeight() * 0.2));
			glm::vec2 cueSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 3.0 + ImGui::GetStyle().WindowPadding.y * 2.0);
			ImGuiWindowFlags cueWindowFlags = ImGuiWindowFlags_NoScrollbar;

			for (auto& manoeuvre : manoeuvres) {
				if(ReorderableList::beginItem(cueSize.y)){
					if(ReorderableList::isItemSelected()) clickedManoeuvre = manoeuvre;
					manoeuvre->listGui();
					ReorderableList::endItem();
				}
			}
			ImGui::PopStyleVar();
			
			ReorderableList::end();
		}
		 
		
		
		//list interaction
		if (clickedManoeuvre) plot->selectManoeuvre(clickedManoeuvre);
		int fromIndex, toIndex;
		if (ReorderableList::wasReordered(fromIndex, toIndex)) {
			manoeuvreList->moveManoeuvre(manoeuvres[fromIndex], toIndex);
		}

		//================== MANOEUVER MANAGER BUTTONS ========================

		if (ImGui::Button("Create", managerButtonSize)) manoeuvreList->addManoeuvre();
		ImGui::SameLine();
		ImGui::BeginDisabled(plot->getSelectedManoeuvre() == nullptr);
		if (ImGui::Button("Duplicate", managerButtonSize)) manoeuvreList->duplicateSelectedManoeuvre();
		ImGui::SameLine();
		if (ImGui::Button("Delete", managerButtonSize)) manoeuvreList->deleteSelectedManoeuvre();
		ImGui::EndDisabled();
		
	}


bool noSelectionDisplay(){
	if (Project::currentPlot->getSelectedManoeuvre() == nullptr) {
		ImGui::PushFont(Fonts::sansRegular20);
		ImGui::Text("%s", "No Manoeuvre Selected");
		ImGui::PopFont();
		return true;
	}
	return false;
}

void trackSheetEditor(){
	if(noSelectionDisplay()) return;
	Project::currentPlot->getSelectedManoeuvre()->sheetEditor();
}

void curveEditor(){
	if(noSelectionDisplay()) return;
	Project::currentPlot->getSelectedManoeuvre()->curveEditor();
}

void spatialEditor(){
	if(noSelectionDisplay()) return;
	Project::currentPlot->getSelectedManoeuvre()->spatialEditor();
}

}
