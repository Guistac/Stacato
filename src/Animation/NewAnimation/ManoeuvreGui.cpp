#include <pch.h>

#include "Manoeuvre.h"

#include <imgui.h>
#include "Gui/Utilities/CustomWidgets.h"
#include "Environnement/Environnement.h"
#include "Animation/NewAnimation/AnimatableRegistry.h"
#include "Animation/NewAnimation/AnimatableOwner.h"
#include "Animation/NewAnimation/Animatable.h"
#include "Animation/NewAnimation/Animation.h"

#include "Gui/Assets/Images.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Fonts.h"

#include "Stacato/StacatoEditor.h"
#include "Stacato/Project/StacatoProject.h"

#include "Gui/Utilities/ReorderableList.h"
#include "Gui/Utilities/CustomWidgets.h"

namespace AnimationSystem{

	void Manoeuvre::editorGui(){

		
		auto currentProject = Stacato::Editor::getCurrentProject();
		
		if(currentProject->isPlotEditLocked()){
			
			ImDrawList* drawing = ImGui::GetWindowDrawList();
			
			ImDrawListSplitter layers;
			layers.Split(drawing, 2);
			layers.SetCurrentChannel(drawing, 1);
			
			float colorStripWidth = ImGui::GetTextLineHeight() * 2.0;
			
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + colorStripWidth + ImGui::GetStyle().FramePadding.x);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y);
			ImGui::PushFont(Fonts::sansBold20);
			ImGui::Text("%s", getName().c_str());
			ImGui::PopFont();
			glm::vec2 titleMin = ImGui::GetItemRectMin();
			glm::vec2 titleMax = ImGui::GetItemRectMax();
			
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + colorStripWidth + ImGui::GetStyle().FramePadding.x);
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::TextWrapped("%s", descriptionParameter->getValue().c_str());
			ImGui::PopStyleColor();
			ImGui::PopFont();
			
			glm::vec2 descriptionMax = ImGui::GetItemRectMax();
			
			layers.SetCurrentChannel(drawing, 0);
			
			glm::vec2 padding = ImGui::GetStyle().FramePadding;
			float maxX = std::max(descriptionMax.x, titleMax.x);
			
			glm::vec2 colorMin = titleMin - padding - glm::vec2(colorStripWidth, 0);
			glm::vec2 colorMax = glm::vec2(colorMin.x + colorStripWidth, descriptionMax.y + padding.y);
			
			glm::vec4 stripColor = glm::vec4(colorStripColor.x, colorStripColor.y, colorStripColor.z, 1.0);
			drawing->AddRectFilled(colorMin, colorMax, ImColor(stripColor), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft);
			drawing->AddRectFilled(titleMin - padding, glm::vec2(maxX, descriptionMax.y) + padding, ImColor(Colors::almostBlack), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersRight);
			drawing->AddRectFilled(titleMin - padding, glm::vec2(maxX, titleMax.y) + padding, ImColor(Colors::veryDarkGray), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersRight);
			
			layers.Merge(drawing);
			
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y);
		}
		else{
			
			float cursorY = ImGui::GetCursorPosY();
			
			ImGui::BeginGroup();
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Name");
			ImGui::PopFont();
			ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 15.0);
			nameParameter->gui();
			ImGui::EndGroup();
			
			ImGui::SetCursorPosY(cursorY);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetItemRectSize().x + ImGui::GetStyle().ItemSpacing.x);
			
			ImGui::BeginGroup();
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Color");
			ImGui::PopFont();
			ImGui::ColorEdit3("##ManoeuvreColor", &colorStripColor.x, ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_NoSidePreview);
			ImGui::EndGroup();
			
			ImGui::BeginGroup();
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Description");
			ImGui::PopFont();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			descriptionParameter->gui();
			ImGui::EndGroup();
			

		}
		
		
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, glm::vec2(ImGui::GetTextLineHeight() * 0.15));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.15));
		ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit;
	
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		float frameHeight = ImGui::GetFrameHeight();
		float iconPadding = ImGui::GetTextLineHeight() * 0.1;
		
			
		for(int i = 0; i < animations->size(); i++){
			auto animation = animations->getEntries()[i];
			ImGui::PushID(i);
			
			ImDrawListSplitter layers;
			layers.Split(drawing, 2);
			layers.SetCurrentChannel(drawing, 1);
			
			ImGui::BeginGroup();
			ImGui::Dummy(glm::vec2(frameHeight,frameHeight));
			drawing->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(Colors::gray), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersTopLeft);
			Image* animationIcon;
			switch(animation->getType()){
				case AnimationType::TARGET: animationIcon = &Images::TargetIcon; break;
				case AnimationType::SEQUENCE: animationIcon = &Images::SequenceIcon; break;
				case AnimationType::STOP: animationIcon = &Images::KeyIcon; break;
			}
			glm::vec2 iconMin = glm::vec2(ImGui::GetItemRectMin()) + glm::vec2(iconPadding,iconPadding);
			glm::vec2 iconMax = glm::vec2(ImGui::GetItemRectMax()) - glm::vec2(iconPadding,iconPadding);
			drawing->AddImage(animationIcon->getID(), iconMin, iconMax, glm::vec2(0,0), glm::vec2(1,1), ImColor(Colors::black));
			ImGui::SameLine(0.0, 0.0);
			backgroundText(animation->getAnimatable()->getOwner()->getName().c_str(), ImVec2(0, frameHeight), Colors::darkGray, Colors::white, ImDrawFlags_RoundCornersNone);
			ImGui::SameLine(0.0, 0.0);
			backgroundText(animation->getAnimatable()->getName().c_str(), ImVec2(0, frameHeight), Colors::veryDarkGray, Colors::white, ImDrawFlags_RoundCornersRight);
			
			if(animation->isCompositeAnimation()){
				auto compositeAnimation = animation->downcasted_shared_from_this<CompositeAnimation>();
				for(int j = 0; j < compositeAnimation->getChildAnimations().size(); j++){
					auto childAnimation = compositeAnimation->getChildAnimations()[j];
					ImGui::PushID(j);
					backgroundText(childAnimation->getAnimatable()->getName().c_str(), ImVec2(0, frameHeight), ImColor(Colors::veryDarkGray), ImColor(Colors::white));
					ImGui::TreePush();
					childAnimation->parameterGui();
					ImGui::TreePop();
					ImGui::PopID();
				}
			}
			else animation->parameterGui();
			ImGui::Dummy(glm::vec2(0,0));
			ImGui::EndGroup();
			
			layers.SetCurrentChannel(drawing, 0);
			glm::vec2 groupMin = ImGui::GetItemRectMin();
			glm::vec2 groupMax = glm::vec2(groupMin.x + ImGui::GetContentRegionAvail().x, ImGui::GetItemRectMax().y);
			drawing->AddRectFilled(groupMin, groupMax, ImColor(Colors::almostBlack), ImGui::GetStyle().FrameRounding);
			layers.Merge(drawing);
			
			ImGui::PopID();
		}
		
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
		
		
		
		
		
		ImGui::PopStyleVar(2);
	}

	void Manoeuvre::listGui(){
		//get drawing coordinates
		glm::vec2 cursor = ImGui::GetCursorPos();
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 size = ImGui::GetItemRectSize();
		glm::vec2 minCursor = ImGui::GetCursorPos();
		bool b_hovered = ReorderableList::isItemHovered();
		//bool b_selected = isSelected();
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		
		glm::vec4 manoeuvreColor = glm::vec4(colorStripColor.x, colorStripColor.y, colorStripColor.z, 1.0);
		float colorStripWidth = ImGui::GetTextLineHeight() * 2.0;
		glm::vec2 maxColorStrip = min + glm::vec2(colorStripWidth, size.y);
		drawing->AddRectFilled(min, maxColorStrip, ImColor(manoeuvreColor), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft);
		
		glm::vec4 backgroundColor = Colors::veryDarkGray;
		//else if selected
		//else if validation error
		glm::vec2 backgroundMin = min + glm::vec2(colorStripWidth, 0.0);
		drawing->AddRectFilled(backgroundMin, max, ImColor(backgroundColor), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersRight);
		
		
		/*
		if(canStop()){
			ImGui::SetItemAllowOverlap();
			if(buttonStop("##stopManoeuvre", imageSize.x)) stop();
		}
		*/
		
		
		
		
		//show name and description
		ImGui::SetCursorPosX(minCursor.x + colorStripWidth + ImGui::GetStyle().FramePadding.x);
		ImGui::SetCursorPosY(minCursor.y + ImGui::GetStyle().FramePadding.y);
		ImGui::PushFont(Fonts::sansBold15);
		float headerTextWidth = ImGui::GetContentRegionAvail().x;
		if(ImGui::CalcTextSize(getName().c_str()).x > headerTextWidth) scrollingText("ScrollingName", getName().c_str(), headerTextWidth, true/*!b_hovered && !b_selected*/, 0.5);
		else ImGui::Text("%s", getName().c_str());
		ImGui::PopFont();
		ImGui::SetCursorPosY(minCursor.y + ImGui::GetTextLineHeightWithSpacing());
		ImGui::SetCursorPosX(minCursor.x + colorStripWidth + ImGui::GetStyle().FramePadding.x);
		glm::vec2 descriptionCursor = ImGui::GetCursorPos();
		ImGui::PushFont(Fonts::sansLight15);
		ImGui::TextWrapped("%s", descriptionParameter->getValue().c_str());
		ImGui::PopFont();
		
		
		
		/*
		if(isSelected()){
		ImGui::SetCursorPosX(descriptionCursor.x);
		ImGui::SetCursorPosY(descriptionCursor.y + ImGui::GetTextLineHeight() * 2.0);
		
			float buttonSize = ImGui::GetTextLineHeight() * 2.0;
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetStyle().ItemSpacing.y));
			
			 switch(getType()){
				 case ManoeuvreType::KEY:{
					 ImGui::BeginDisabled(!canRapidToPlaybackPosition());
					 bool atKey = isAtPlaybackPosition();
					 if(atKey) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					 ImGui::SetItemAllowOverlap();
					 if(buttonArrowDownStop("goToPos", buttonSize)) rapidToPlaybackPosition();
					 if(atKey) ImGui::PopStyleColor();
					 ImGui::EndDisabled();
				 }break;
				 case ManoeuvreType::TARGET:{
					 ImGui::BeginDisabled(!canRapidToTarget());
					 bool atTarget = isAtTarget();
					 if(atTarget) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					 ImGui::SetItemAllowOverlap();
					 if(buttonArrowRightStop("goToTarget", buttonSize)) rapidToTarget();
					 if(atTarget) ImGui::PopStyleColor();
					 ImGui::EndDisabled();
					 ImGui::SameLine();
					 ImGui::BeginDisabled(!canStartPlayback());
					 ImGui::SetItemAllowOverlap();
					 if(buttonPlay("StartPlayback", buttonSize)) startPlayback();
					 ImGui::EndDisabled();
				 }break;
				 case ManoeuvreType::SEQUENCE:{
					 ImGui::BeginDisabled(!canRapidToStart());
					 bool atStart = isAtStart();
					 if(atStart) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					 ImGui::SetItemAllowOverlap();
					 if(buttonArrowLeftStop("goToStart", buttonSize)) rapidToStart();
					 if(atStart) ImGui::PopStyleColor();
					 ImGui::EndDisabled();
					 ImGui::SameLine();
					 ImGui::BeginDisabled(!canRapidToTarget());
					 bool atTarget = isAtTarget();
					 if(atTarget) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					 ImGui::SetItemAllowOverlap();
					 if(buttonArrowRightStop("goToTarget", buttonSize)) rapidToTarget();
					 if(atTarget) ImGui::PopStyleColor();
					 ImGui::EndDisabled();
					 ImGui::SameLine();
					 if(canPausePlayback()){
						 ImGui::BeginDisabled(!canPausePlayback());
						 ImGui::SetItemAllowOverlap();
						 if(buttonPause("PausePlayback", buttonSize)) pausePlayback();
						 ImGui::EndDisabled();
					 }
					 else{
						 ImGui::BeginDisabled(!canStartPlayback());
						 ImGui::SetItemAllowOverlap();
						 if(buttonPlay("StartPlayback", buttonSize)) startPlayback();
						 ImGui::EndDisabled();
					 }
				 }break;
			 }
			ImGui::PopStyleVar();
		
		}
		*/
		
		
		
		//show playback or rapid state
		
		/*
		if(b_hasActiveAnimations){
			auto activeAnimations = getActiveAnimations();
			int animationCount = activeAnimations.size();
			float animationHeight = size.y / (float)animationCount;
			for(int i = 0; i < animationCount; i++){
				auto& animation = activeAnimations[i];
				float progress;
				if(animation->isInRapid()) progress = animation->getRapidProgress();
				else progress = animation->getPlaybackProgress();
				progress = std::clamp(progress, 0.01f, 1.0f);
				glm::vec2 minBar(min.x, min.y + animationHeight * i);
				glm::vec2 maxBar(minBar.x + size.x * progress, minBar.y + animationHeight);
				ImGui::GetWindowDrawList()->AddRectFilled(minBar, maxBar, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.1)), 5.0);
			}
			
		}
		*/
		
		//mouse interaction and selection display
		/*
		if(b_hovered && !ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
			glm::vec4 color;
			if(ImGui::IsMouseDown(ImGuiMouseButton_Left)) color = glm::vec4(0.f, 0.f, 0.f, .3f);
			else color = glm::vec4(1.f, 1.f, 1.f, .1f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(color), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		}
		 */
		/*
		if(b_selected){
			float thickness = ImGui::GetTextLineHeight() * 0.1;
			float rounding = ImGui::GetStyle().FrameRounding - thickness / 2.0;
			glm::vec2 minSelection = min + glm::vec2(thickness * .5f);
			glm::vec2 maxSelection = max - glm::vec2(thickness * .5f);
			ImGui::GetWindowDrawList()->AddRect(minSelection, maxSelection, ImColor(1.0f, 1.0f, 1.0f, .5f), rounding, ImDrawFlags_RoundCornersAll, thickness);
		}
		*/
	}

};
