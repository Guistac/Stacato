#include <pch.h>

#include "Manoeuvre.h"

#include <imgui.h>
#include "Gui/Utilities/CustomWidgets.h"
#include "Environnement/Environnement.h"
#include "Animation/NewAnimation/AnimatableRegistry.h"
#include "Animation/NewAnimation/AnimatableOwner.h"
#include "Animation/NewAnimation/Animatable.h"

#include "Gui/Assets/Images.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Fonts.h"

#include "Stacato/StacatoEditor.h"
#include "Stacato/Project/StacatoProject.h"

namespace AnimationSystem{

	void Manoeuvre::editorGui(){

		
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, glm::vec2(ImGui::GetTextLineHeight() * 0.15));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.15));
		ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX;
	

		if(ImGui::BeginTable("##animations", 2, tableFlags)){
			
			ImGui::TableSetupColumn("Animatable");
			ImGui::TableSetupColumn("Parameters");
			ImGui::TableHeadersRow();
			
			for(int i = 0; i < animations.size(); i++){
				auto animation = animations[i];
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Name of Animatable");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("Animation Parameters");
				
			}
			
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if(ImGui::Button("Add Animation")) ImGui::OpenPopup("AnimatableSelector");
			
			
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
			
			auto animationSelector = [&](std::shared_ptr<Animatable> animatable) -> bool {
				for(auto animationType : animatable->getSupportedAnimationTypes()){
					switch(animationType){
						case AnimationType::TARGET:
							if(manoeuvreTypeSelector(Images::TargetIcon, "Target", 300)){
								addAnimation(animatable->makeAnimation(animationType));
								return true;
							}
							break;
						case AnimationType::SEQUENCE:
							if(manoeuvreTypeSelector(Images::SequenceIcon, "Sequence", 300)){
								addAnimation(animatable->makeAnimation(animationType));
								return true;
							}
							break;
						case AnimationType::STOP:
							if(manoeuvreTypeSelector(Images::KeyIcon, "Stop", 300)){
								addAnimation(animatable->makeAnimation(animationType));
								return true;
							}
							break;
					}
				}
				return false;
			};
			
			if (ImGui::BeginPopup("AnimatableSelector")) {
				
				auto project = Stacato::Editor::getCurrentProject();
				auto& animatableOwners = project->getEnvironnement()->getAnimatableRegistry()->getAnimatableOwners();
				
				for (int i = 0; i < animatableOwners.size(); i++) {
					auto animatableOwner = animatableOwners[i];
					auto& animatables = animatableOwner->getAnimatables();
					if(animatables.empty()) continue;
					
					ImGui::PushID(i);
					
					if(ImGui::BeginMenu(animatableOwner->getName().c_str())){
					
						auto& animatables = animatableOwner->getAnimatables();
						if(animatables.size() == 1){
							if(animationSelector(animatables[0])) ImGui::CloseCurrentPopup();
						}
						else if(!animatables.empty()){
							for(int j = 0; j < animatables.size(); j++){
								auto animatable = animatables[j];
								if(animationSelector(animatable)) ImGui::CloseCurrentPopup();
							}
						}
						
						ImGui::EndMenu();
					}
					
					
					
					ImGui::PopID();
				}
				ImGui::EndPopup();
			}
			
			
			
			ImGui::EndTable();
		}
		
		
		
		
		
		ImGui::PopStyleVar(2);
	}

};
