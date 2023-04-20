#include <pch.h>

#include <imgui.h>

#include "Gui/Utilities/ReorderableList.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Plot/Plot.h"
#include "Animation/Manoeuvre.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Animation/Playback/Playback.h"

#include "Plot/ManoeuvreList.h"

#include "Gui/Utilities/ReorderableList.h"

#include "PlotGui.h"

#include "Gui/Assets/Images.h"

#include "Stacato/StacatoEditor.h"
#include "Stacato/Project/StacatoProject.h"


#include "Animation/NewAnimation/Manoeuvre.h"

namespace PlotGui{

	void manoeuvreList() {
		
		if(!Stacato::Editor::hasCurrentProject()){
			ImGui::Text("No Project Loaded");
			return;
		}
		
		auto currentProject = Stacato::Editor::getCurrentProject();
		auto currentPlot = currentProject->getPlot();
		auto& manoeuvreLists = currentPlot->getManoeuvreLists();
		auto selectedManoeuvreList = currentPlot->getSelectedManoeuvreList();
		
		//================= MANOEUVRE LIST =======================

		float width = ImGui::GetContentRegionAvail().x;
		
		ImGui::PushFont(Fonts::sansBold20);
		float titleBarHeight = ImGui::GetFrameHeight();
		if(customButton(selectedManoeuvreList->getName().c_str(),
						glm::vec2(width, ImGui::GetFrameHeight()),
						Colors::darkGray,
						ImGui::GetStyle().FrameRounding,
						ImDrawFlags_RoundCornersAll)){
			ImGui::OpenPopup("PlotSelector");
		}
		ImGui::PopFont();
		
		ImGui::SetNextWindowPos(ImGui::GetItemRectMin());
		ImGui::PushStyleColor(ImGuiCol_PopupBg, Colors::almostBlack);
		if(ImGui::BeginPopup("PlotSelector")){
			ImGui::Dummy(glm::vec2(width - ImGui::GetStyle().WindowPadding.x * 2.0, 1));
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::Text("Select Plot: ");
			ImGui::PopStyleColor();
			ImGui::PopFont();
			ImGui::Separator();
			for(int i = 0; i < manoeuvreLists.size(); i++){
				auto manoeuvreList = manoeuvreLists[i];
				ImGui::PushID(i);
				bool b_isCurrent = manoeuvreList == selectedManoeuvreList;
				if(b_isCurrent) {
					ImGui::PushFont(Fonts::sansBold15);
					ImGui::PushStyleColor(ImGuiCol_Text, Colors::yellow);
				}
                if(ImGui::Selectable(manoeuvreList->getName().c_str())) {
					currentPlot->selectManoeuvreList(manoeuvreList);
                }
				if(b_isCurrent) {
					ImGui::PopFont();
					ImGui::PopStyleColor();
				}
				ImGui::PopID();
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleColor();
		
		
		float managerButtonWidth = (ImGui::GetContentRegionAvail().x - 2.0) / 3.0;
		float managerButtonHeight = ImGui::GetTextLineHeight() * 2.0;
		ImVec2 managerButtonSize(managerButtonWidth, managerButtonHeight);
		
		float managerHeight = managerButtonSize.y + ImGui::GetStyle().ItemSpacing.y;
		glm::vec2 manoeuvreListSize = ImGui::GetContentRegionAvail();
        if(!currentProject->isPlotEditLocked()) manoeuvreListSize.y -= managerHeight;
		
		auto& manoeuvres = selectedManoeuvreList->getManoeuvres();
		std::shared_ptr<AnimationSystem::Manoeuvre> clickedManoeuvre = nullptr;
		
		if(ReorderableList::begin("CueList", manoeuvreListSize, !currentProject->isPlotEditLocked())){
		
			//0 horizontal padding is to display the header background strip up to the edge of the cue window
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0, ImGui::GetTextLineHeight() * 0.2));
			
			float controlButtonSize = ImGui::GetTextLineHeight() * 2.0;
			float cueSizeY = ImGui::GetTextLineHeight() * 3.0 + ImGui::GetStyle().WindowPadding.y * 2.0;
			float cueSizeSelectedY = cueSizeY + controlButtonSize + ImGui::GetStyle().ItemSpacing.y;
			
			if(ImGui::IsWindowFocused()){
				if(ImGui::IsKeyPressed(GLFW_KEY_UP)) {
					//manoeuvreList->selectPreviousManoeuvre();
				}
				if(ImGui::IsKeyPressed(GLFW_KEY_DOWN)) {
					//manoeuvreList->selectNextManoeuvre();
				}
			}
			
			ImGuiWindow* list = ImGui::GetCurrentWindow();
			
			
			for (auto& manoeuvre : manoeuvres) {
				
				if(ReorderableList::beginItem(cueSizeY)){
					ImGui::Text("Manoeuvre");
					/*
					if(plot->b_scrollToSelectedManoeuvre && manoeuvre->isSelected()){
						ImGui::SetScrollHereY(.5f);
						plot->b_scrollToSelectedManoeuvre = false;
					}
					*/
					
					if(ReorderableList::isItemSelected()) clickedManoeuvre = manoeuvre;
					//manoeuvre->listGui();
					ReorderableList::endItem();
				}
				/*
				else if(plot->b_scrollToSelectedManoeuvre && manoeuvre->isSelected()){
					ImGui::SetScrollHereY(.5f);
					plot->b_scrollToSelectedManoeuvre = false;
				}
				 */
				 
			}
			ImGui::PopStyleVar();
			
			if(manoeuvres.empty()){
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("No manoeuvres.");
				ImGui::PopFont();
				ImGui::TextWrapped("Add new manoeuvres using the controls at the bottom of this window.");
				ImGui::PopStyleColor();
			}
			 
			
			ReorderableList::end();
		}
		 
		
		
		//list interaction
		if (clickedManoeuvre) currentPlot->selectManoeuvre(clickedManoeuvre);
		int fromIndex, toIndex;
		if (ReorderableList::wasReordered(fromIndex, toIndex)) {
			selectedManoeuvreList->moveManoeuvre(manoeuvres[fromIndex], toIndex);
		}

        
        if(!currentProject->isPlotEditLocked()){
        
            //================== MANOEUVER MANAGER BUTTONS ========================

            
            float buttonRounding = ImGui::GetStyle().FrameRounding;
            ImVec4 defaultButtonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
            
            
            static glm::vec2 createPopupPosition;
            
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(1.0));
            
            if(customButton("Create", managerButtonSize, defaultButtonColor, buttonRounding, ImDrawFlags_RoundCornersLeft)){
				selectedManoeuvreList->addManoeuvre();
            }
            
            float popupWidth = ImGui::GetTextLineHeight() * 5.0;
            float selectorHeight = ImGui::GetFrameHeight();
            glm::vec2 popupSize(popupWidth, selectorHeight * 3.0 + ImGui::GetStyle().WindowBorderSize * 2.0);
            

                        
            ImGui::SameLine();
            
            if(customButton("Duplicate", managerButtonSize, defaultButtonColor, buttonRounding, ImDrawFlags_RoundCornersNone)){
				selectedManoeuvreList->duplicateSelectedManoeuvre();
            }
            
            ImGui::SameLine();
            
            static bool b_waitingForDeleteConfirmation = false;
            if(b_waitingForDeleteConfirmation){
                ImVec4 confirmColor = Timing::getBlink(.5) ? Colors::red : Colors::darkRed;
                if(customButton("Confirm", managerButtonSize, confirmColor, buttonRounding, ImDrawFlags_RoundCornersRight)){
					selectedManoeuvreList->deleteSelectedManoeuvre();
                }
                if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) b_waitingForDeleteConfirmation = false;
            }else{
                if(customButton("Delete", managerButtonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], buttonRounding, ImDrawFlags_RoundCornersRight)) b_waitingForDeleteConfirmation = true;
            }
             
            
            ImGui::PopStyleVar();
            
        }
		
	}

	bool noSelectionDisplay(){
		
		if(!Stacato::Editor::hasCurrentProject()){
			ImGui::Text("No Project Loaded");
			return true;
		}
		
		if (Stacato::Editor::getCurrentProject()->getPlot()->getSelectedManoeuvre() == nullptr) {
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("No Manoeuvre Selected.");
			ImGui::PopFont();
			ImGui::TextWrapped("Select manoeuvres in the manoeuvre list.");
			ImGui::PopStyleColor();
			return true;
		}
		return false;
	}

/*

void curveEditor(){
	if(noSelectionDisplay()) return;
	//Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre()->curveEditor();
}

void spatialEditor(){
	if(noSelectionDisplay()) return;
	//Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre()->spatialEditor();
}
*/

void trackSheetEditor(){
	if(noSelectionDisplay()) return;
	//Stacato::Editor::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre()->sheetEditor();
	Stacato::Editor::getCurrentProject()->getPlot()->getSelectedManoeuvre()->editorGui();
}


void ManoeuvreListCreationPopup::open(std::shared_ptr<Plot> plot){
	static auto popup = std::make_shared<ManoeuvreListCreationPopup>();
	popup->targetPlot = plot;
	popup->Legato::Popup::open();
	popup->nameParameter->overwrite("New Manoeuvre List");
}

void ManoeuvreListCreationPopup::onDraw(){
	ImGui::Text("Enter a name for the new Manoeuvre List:");
	nameParameter->gui();
	if(ImGui::Button("Confirm")){
		auto newManoeuvreList = ManoeuvreList::createInstance();
		newManoeuvreList->setName(nameParameter->getValue());
		targetPlot->addManoeuvreList(newManoeuvreList);
		close();
	}
	ImGui::SameLine();
	if(ImGui::Button("Cancel")){
		close();
	}
}


void ManoeuvreListEditPopup::open(std::shared_ptr<ManoeuvreList> manoeuvreList){
	static auto popup = std::make_shared<ManoeuvreListEditPopup>();
	popup->editedManoeuvreList = manoeuvreList;
	popup->Legato::Popup::open();
	popup->nameParameter->overwrite(manoeuvreList->getName());
}

void ManoeuvreListEditPopup::onDraw(){
	ImGui::Text("Manoeuvre List Name:");
	nameParameter->gui();
	if(ImGui::Button("Confirm")){
		editedManoeuvreList->setName(nameParameter->getValue());
		close();
	}
	ImGui::SameLine();
	if(ImGui::Button("Cancel")){
		close();
	}
}

void ManoeuvreListDeletePopup::open(std::shared_ptr<Plot> plot, std::shared_ptr<ManoeuvreList> manoeuvreList){
	static auto popup = std::make_shared<ManoeuvreListDeletePopup>();
	popup->deletedManoeuvreList = manoeuvreList;
	popup->targetPlot = plot;
	popup->Popup::open();
}

void ManoeuvreListDeletePopup::onDraw(){
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
	ImGui::Text("Do you really want to delete %s ?", deletedManoeuvreList->getName().c_str());
	ImGui::PopStyleColor();
	ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkRed);
	if(ImGui::Button("Delete")) {
		targetPlot->removeManoeuvreList(deletedManoeuvreList);
		close();
	}
	ImGui::PopStyleColor();
	ImGui::SameLine();
	if(ImGui::Button("Cancel")){
		close();
	}
}

}


