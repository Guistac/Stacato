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

#include "Stacato/StacatoWorkspace.h"
#include "Stacato/StacatoProject.h"


namespace PlotGui{

	void manoeuvreList() {
		
		if(!Stacato::Workspace::hasCurrentProject()){
			ImGui::Text("No Project Loaded");
			return;
		}
		
		auto currentProject = Stacato::Workspace::getCurrentProject();
		
		//================= MANOEUVRE LIST =======================

		std::shared_ptr<Plot> plot = Stacato::Workspace::getCurrentProject()->getCurrentPlot();
		float width = ImGui::GetContentRegionAvail().x;
		
		ImGui::PushFont(Fonts::sansBold20);
		float titleBarHeight = ImGui::GetFrameHeight();
		if(customButton(plot->getName(),
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
			auto& plots = Stacato::Workspace::getCurrentProject()->getPlots();
			for(int i = 0; i < plots.size(); i++){
				auto plot = plots[i];
				ImGui::PushID(i);
				bool b_isCurrent = plot->isCurrent();
				if(b_isCurrent) {
					ImGui::PushFont(Fonts::sansBold15);
					ImGui::PushStyleColor(ImGuiCol_Text, Colors::yellow);
				}
                if(ImGui::Selectable(plot->getName())) {
					Stacato::Workspace::getCurrentProject()->setCurrentPlot(plot);
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
		
		std::shared_ptr<ManoeuvreList> manoeuvreList = plot->getManoeuvreList();
		std::vector<std::shared_ptr<Manoeuvre>>& manoeuvres = manoeuvreList->getManoeuvres();
		std::shared_ptr<Manoeuvre> clickedManoeuvre = nullptr;
		
		if(ReorderableList::begin("CueList", manoeuvreListSize, !currentProject->isPlotEditLocked())){
		
			//0 horizontal padding is to display the header background strip up to the edge of the cue window
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0, ImGui::GetTextLineHeight() * 0.2));
			
			float controlButtonSize = ImGui::GetTextLineHeight() * 2.0;
			float cueSizeY = ImGui::GetTextLineHeight() * 3.0 + ImGui::GetStyle().WindowPadding.y * 2.0;
			float cueSizeSelectedY = cueSizeY + controlButtonSize + ImGui::GetStyle().ItemSpacing.y;
			
			if(ImGui::IsWindowFocused()){
				if(ImGui::IsKeyPressed(GLFW_KEY_UP)) {
					manoeuvreList->selectPreviousManoeuvre();
				}
				if(ImGui::IsKeyPressed(GLFW_KEY_DOWN)) {
					manoeuvreList->selectNextManoeuvre();
				}
			}
			
			ImGuiWindow* list = ImGui::GetCurrentWindow();
			
			for (auto& manoeuvre : manoeuvres) {
				if(ReorderableList::beginItem(manoeuvre->isSelected() ? cueSizeSelectedY : cueSizeY)){
					if(plot->b_scrollToSelectedManoeuvre && manoeuvre->isSelected()){
						ImGui::SetScrollHereY(.5f);
						plot->b_scrollToSelectedManoeuvre = false;
					}
					
					if(ReorderableList::isItemSelected()) clickedManoeuvre = manoeuvre;
					manoeuvre->listGui();
					ReorderableList::endItem();
				}else if(plot->b_scrollToSelectedManoeuvre && manoeuvre->isSelected()){
					ImGui::SetScrollHereY(.5f);
					plot->b_scrollToSelectedManoeuvre = false;
				}
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
		if (clickedManoeuvre) plot->selectManoeuvre(clickedManoeuvre);
		int fromIndex, toIndex;
		if (ReorderableList::wasReordered(fromIndex, toIndex)) {
			manoeuvreList->moveManoeuvre(manoeuvres[fromIndex], toIndex);
		}

        
        if(!currentProject->isPlotEditLocked()){
        
            //================== MANOEUVER MANAGER BUTTONS ========================

            
            float buttonRounding = ImGui::GetStyle().FrameRounding;
            ImVec4 defaultButtonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
            
            
            static glm::vec2 createPopupPosition;
            
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(1.0));
            
            if(customButton("Create", managerButtonSize, defaultButtonColor, buttonRounding, ImDrawFlags_RoundCornersLeft)){
                ImGui::OpenPopup("Create Manoeuvre");
                createPopupPosition = ImGui::GetMousePos();
            }
            
            float popupWidth = ImGui::GetTextLineHeight() * 5.0;
            float selectorHeight = ImGui::GetFrameHeight();
            glm::vec2 popupSize(popupWidth, selectorHeight * 3.0 + ImGui::GetStyle().WindowBorderSize * 2.0);
            
            ImGui::SetNextWindowPos(createPopupPosition + glm::vec2(0.0, -popupSize.y));
            ImGui::SetNextWindowSize(popupSize);
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
		
	}


bool noSelectionDisplay(){
	
	if(!Stacato::Workspace::hasCurrentProject()){
		ImGui::Text("No Project Loaded");
		return true;
	}
	
	if (Stacato::Workspace::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre() == nullptr) {
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

void trackSheetEditor(){
	if(noSelectionDisplay()) return;
	Stacato::Workspace::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre()->sheetEditor();
}

void curveEditor(){
	if(noSelectionDisplay()) return;
	Stacato::Workspace::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre()->curveEditor();
}

void spatialEditor(){
	if(noSelectionDisplay()) return;
	Stacato::Workspace::getCurrentProject()->getCurrentPlot()->getSelectedManoeuvre()->spatialEditor();
}


void NewPlotPopup::onOpen(){
	sprintf(newNameBuffer, "New Plot");
}

void NewPlotPopup::onDraw(){
	ImGui::Text("Enter a name for the new Plot:");
	ImGui::InputText("##plotName", newNameBuffer, 256);
	if(ImGui::Button("Confirm")){
		auto newPlot = Stacato::Workspace::getCurrentProject()->createNewPlot();
		newPlot->setName(newNameBuffer);
		close();
	}
	ImGui::SameLine();
	if(ImGui::Button("Cancel")){
		close();
	}
}


void PlotEditorPopup::onOpen(){
	strcpy(newNameBuffer, plot->getName());
}

void PlotEditorPopup::onDraw(){
	ImGui::Text("Plot Name:");
	ImGui::InputText("##plotName", newNameBuffer, 256);
	if(ImGui::Button("Confirm")){
		plot->setName(newNameBuffer);
		close();
	}
	ImGui::SameLine();
	if(ImGui::Button("Cancel")){
		close();
	}
}


void PlotDeletePopup::onDraw(){
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
	ImGui::Text("Do you really want to delete %s ?", plot->getName());
	ImGui::PopStyleColor();
	ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkRed);
	if(ImGui::Button("Delete")) {
		Stacato::Workspace::getCurrentProject()->deletePlot(plot);
		close();
	}
	ImGui::PopStyleColor();
	ImGui::SameLine();
	if(ImGui::Button("Cancel")){
		close();
	}
}



}
