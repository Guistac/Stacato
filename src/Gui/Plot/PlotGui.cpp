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

#include "PlotGui.h"

#include "Gui/Assets/Images.h"

namespace PlotGui{

	void manoeuvreList() {

		float managerButtonWidth = (ImGui::GetContentRegionAvail().x - 2.0) / 3.0;
		float managerButtonHeight = ImGui::GetTextLineHeight() * 2.0;
		ImVec2 managerButtonSize(managerButtonWidth, managerButtonHeight);
		
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

		
		float buttonRounding = ImGui::GetStyle().FrameRounding;
		ImVec4 defaultButtonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
		
		
		static ImVec2 createPopupPosition;
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(1.0));
		
		if(customButton("Create", managerButtonSize, defaultButtonColor, buttonRounding, ImDrawFlags_RoundCornersLeft)){
			ImGui::OpenPopup("Create Manoeuvre");
			createPopupPosition = ImGui::GetMousePos();
		}
		
		ImGui::SetNextWindowPos(createPopupPosition);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0.0));
		if(ImGui::BeginPopup("Create Manoeuvre")){
			
			auto manoeuvreTypeSelector = [](Image& image, const char* txt, float width) -> bool {
				float height = ImGui::GetFrameHeight();
				
				ImGui::PushID(txt);
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.0f, .0f, .0f, .0f));
				bool b_pressed = ImGui::Button("##", glm::vec2(width, height));
				ImGui::PopStyleColor();
				ImGui::PopID();
				
				glm::vec2 buttonPos = ImGui::GetItemRectMin();
				glm::vec2 imageShrink(height * .1f);
				glm::vec2 imageMin = buttonPos + imageShrink;
				glm::vec2 imageMax = buttonPos + glm::vec2(height) - imageShrink;
				
				ImDrawList* drawing = ImGui::GetWindowDrawList();
				drawing->AddImage(image.getID(), imageMin, imageMax);
				glm::vec2 textPos(buttonPos.x + height, buttonPos.y + (height - ImGui::GetTextLineHeight()) / 2.0);
				drawing->AddText(textPos, ImColor(Colors::white), txt);
				
				return b_pressed;
			};
			
			float popupWidth = ImGui::GetTextLineHeight() * 6.0;
			
			if(manoeuvreTypeSelector(Images::KeyIcon, "Key", popupWidth)) {
				manoeuvreList->addManoeuvre(ManoeuvreType::KEY);
				ImGui::CloseCurrentPopup();
			}
			if(manoeuvreTypeSelector(Images::TargetIcon, "Target", popupWidth)) {
				manoeuvreList->addManoeuvre(ManoeuvreType::TARGET);
				ImGui::CloseCurrentPopup();
			}
			if(manoeuvreTypeSelector(Images::SequenceIcon, "Sequence", popupWidth)) {
				manoeuvreList->addManoeuvre(ManoeuvreType::SEQUENCE);
				ImGui::CloseCurrentPopup();
			}
			
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
		
		
		ImGui::SameLine();
		
		if(customButton("Duplicate", managerButtonSize, defaultButtonColor, buttonRounding, ImDrawFlags_RoundCornersNone)){
			manoeuvreList->duplicateSelectedManoeuvre();
		}
		
		ImGui::SameLine();
		
		static bool b_waitingForDeleteConfirmation = false;
		if(b_waitingForDeleteConfirmation){
			ImVec4 confirmColor = Timing::getBlink(.5) ? Colors::red : Colors::darkRed;
			if(customButton("Confirm", managerButtonSize, confirmColor, buttonRounding, ImDrawFlags_RoundCornersRight)){
				manoeuvreList->deleteSelectedManoeuvre();
			}
			if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) b_waitingForDeleteConfirmation = false;
		}else{
			if(customButton("Delete", managerButtonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], buttonRounding, ImDrawFlags_RoundCornersRight)) b_waitingForDeleteConfirmation = true;
		}
		 
		
		ImGui::PopStyleVar();
		
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
