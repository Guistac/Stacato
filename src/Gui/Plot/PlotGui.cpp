#include <pch.h>

//#include "ProjectGui.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Utilities/DraggableList.h"
#include "Gui/Utilities/CustomWidgets.h"


#include "Project/Project.h"
#include "Plot/Plot.h"
#include "Motion/Manoeuvre/Manoeuvre.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Motion/Playback.h"

namespace PlotGui{

	void editor() {

		static float minSideBarWidth = ImGui::GetTextLineHeight() * 18.0;
		static float sideBarWidth = ImGui::GetTextLineHeight() * 18.0;
		ImGui::BeginChild("##ManoeuvreList", glm::vec2(sideBarWidth, ImGui::GetContentRegionAvail().y), false);

		std::shared_ptr<Plot> currentPlot = Project::currentPlot;
		std::shared_ptr<Manoeuvre> selectedManoeuvre = currentPlot->getSelectedManoeuvre();

		float plotHeaderHeight = ImGui::GetTextLineHeight() * 2.5;
		//glm::vec2 plotEditSwitchButtonSize(ImGui::GetTextLineHeight() * 5.0, plotHeaderHeight);
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
		
		
		
		ImGui::PushFont(Fonts::robotoBold20);
		BackgroundText::draw("Conduite"/*currentPlot->name*/, glm::vec2(sideBarWidth, plotHeaderHeight), Colors::darkGray);
		ImGui::PopFont();
		
		/*
		ImGui::SameLine();
		static ToggleSwitch editSwitch;
		bool b_plotEditAllowed = Project::isPlotEditAllowed();
		if(editSwitch.draw("##EditToggleSwitch", b_plotEditAllowed, "Editing", "Playback", plotEditSwitchButtonSize))
			Project::setPlotEdit(!b_plotEditAllowed);
		 */
		
		ImGui::PopStyleVar();

		glm::vec2 managerButtonSize((sideBarWidth - ImGui::GetStyle().ItemSpacing.x * 2.0) / 3.0, ImGui::GetTextLineHeight() * 2.0);
		float bottomSectionHeight = ImGui::GetTextLineHeightWithSpacing() * 2.0;	//two title text lines
		bottomSectionHeight += managerButtonSize.y * 2.0;							//two button rows (manager buttons & stop buttons)
		bottomSectionHeight += ImGui::GetStyle().ItemSpacing.y * 2.0;				//spacing for the two button rows
		if (selectedManoeuvre) bottomSectionHeight += Manoeuvre::getPlaybackControlGuiHeight(selectedManoeuvre);
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
					Manoeuvre::listGui(manoeuvre);
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
			currentPlot->reorderManoeuvre(manoeuvres[oldIndex], oldIndex, newIndex);
		}


		//================== MANOEUVER MANAGER BUTTONS ========================

		static const char* manoeuvreManagerString = "Manoeuvre Manager";
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::NewLine();
		ImGui::SameLine((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(manoeuvreManagerString).x) / 2.0);
		ImGui::Text("%s", manoeuvreManagerString);
		ImGui::PopFont();

		if (ImGui::Button("Create", managerButtonSize)) currentPlot->addManoeuvre();
		ImGui::SameLine();
		bool disablSelectionBasedButtons = selectedManoeuvre == nullptr;
		ImGui::BeginDisabled(disablSelectionBasedButtons);
		if (ImGui::Button("Duplicate", managerButtonSize)) currentPlot->duplicateSelectedManoeuvre();
		ImGui::SameLine();
		if (ImGui::Button("Delete", managerButtonSize)) currentPlot->deleteSelectedManoeuvre();
		ImGui::EndDisabled();

		static const char* playbackControlsString = "Playback Controls";
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::NewLine();
		ImGui::SameLine((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(playbackControlsString).x) / 2.0);
		ImGui::Text("%s", playbackControlsString);
		ImGui::PopFont();

		//===================== PLAYBACK MANAGER BUTTONS =======================

		static const char* noManoeuvreSelectedString = "No Manoeuvre Selected";

		if (selectedManoeuvre) {
			Manoeuvre::playbackControlGui(selectedManoeuvre);
		}
		else {
			ImGui::NewLine();
			ImGui::SameLine((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(noManoeuvreSelectedString).x) / 2.0);
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::Text("No Manoeuvre Selected");
			ImGui::PopStyleColor();
		}

		if (ImGui::Button("Stop All", glm::vec2(ImGui::GetContentRegionAvail().x, managerButtonSize.y))) Playback::stopAllManoeuvres();

		ImGui::EndChild();

		static float splitterWidth = ImGui::GetTextLineHeight() * 0.5;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::SameLine();
		ImGui::InvisibleButton("VerticalSplitter", glm::vec2(splitterWidth, ImGui::GetContentRegionAvail().y));
		if (ImGui::IsItemActive()) sideBarWidth += ImGui::GetIO().MouseDelta.x;
		if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		if (sideBarWidth < minSideBarWidth) sideBarWidth = minSideBarWidth;
		
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		float middle = (min.x + max.x) / 2.0;
		ImGui::GetWindowDrawList()->AddLine(glm::vec2(middle, min.y), glm::vec2(middle, max.y), ImColor(Colors::darkGray), ImGui::GetTextLineHeight() * 0.1);
		ImGui::SameLine();
		ImGui::PopStyleVar();

		if (ImGui::BeginChild("##CueProperties", ImGui::GetContentRegionAvail(), false)) {
		
			if (currentPlot->selectedManoeuvre == nullptr) {
				ImGui::PushFont(Fonts::robotoRegular20);
				ImGui::Text("%s", noManoeuvreSelectedString);
				ImGui::PopFont();
			}
			else {
				std::shared_ptr<Manoeuvre> selectedManoeuvre = currentPlot->selectedManoeuvre;
				Manoeuvre::editGui(selectedManoeuvre);
			}
		}
		ImGui::EndChild();

		//we need to change selection after drawing the previously selected manoeuvre
		//else data might get wrongly copied
		if (clickedManoeuvre) currentPlot->selectManoeuvre(clickedManoeuvre);

	}

}
