#include <pch.h>

#include "Animation/Manoeuvre.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Images.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"
#include "Animation/Animatable.h"
#include "Animation/Animation.h"
#include "Motion/Curve/Curve.h"
#include "Plot/Plot.h"

#include "Animation/Playback/Playback.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Gui/Utilities/ReorderableList.h"

#include "Stacato/Stacato.h"
#include "Stacato/StacatoProject.h"


//inside draggable list element: InvisibleButton/ClipRectangle
void Manoeuvre::listGui(){
	
	//get drawing coordinates
	glm::vec2 cursor = ImGui::GetCursorPos();
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	glm::vec2 size = ImGui::GetItemRectSize();
	glm::vec2 minCursor = ImGui::GetCursorPos();
	bool b_hovered = ReorderableList::isItemHovered();
	bool b_selected = isSelected();
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	//draw background and show validness
	ImColor backgroundColor;
	if(b_selected) backgroundColor = ImColor(Colors::blue);
	else backgroundColor = ImColor(Colors::veryDarkGray);
	drawing->AddRectFilled(min, max, backgroundColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	if(!b_valid){
		ImColor blinkColor;
		if(Timing::getBlink(1.0)) blinkColor = ImColor(1.f, 0.f, 0.f, .8f);
		else blinkColor = ImColor(1.f, 0.f, 0.f, .2f);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, blinkColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	}
	
	//show header strip with icon
	float headerStripWidth = ImGui::GetTextLineHeight() * 2.3;
	glm::vec2 maxHeaderStrip = min + glm::vec2(headerStripWidth, ImGui::GetItemRectSize().y);
	glm::vec4 headerStripColor;
	const char* manoeuvreTypeString;
	Image* icon;
	switch (type->value) {
		case ManoeuvreType::KEY:
			headerStripColor = Colors::darkYellow;
			manoeuvreTypeString = "KEY";
			icon = &Images::KeyIcon;
			break;
		case ManoeuvreType::TARGET:
			headerStripColor = Colors::darkGray;
			manoeuvreTypeString = "TAR";
			icon = &Images::TargetIcon;
			break;
		case ManoeuvreType::SEQUENCE:
			headerStripColor = Colors::darkRed;
			manoeuvreTypeString = "SEQ";
			icon = &Images::SequenceIcon;
			break;
	}
	
	
	drawing->AddRectFilled(min, maxHeaderStrip, ImColor(headerStripColor), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft);
	ImGui::PushFont(Fonts::sansRegular20);
	ImColor textColor;
	if(b_selected) textColor = ImColor(1.f, 1.f, 1.f, 1.f);
	else textColor = ImColor(.0f, 0.f, 0.f, .9f);
	glm::vec2 textSize = ImGui::CalcTextSize(manoeuvreTypeString);
	
	glm::vec2 headerTextPos(min.x + (headerStripWidth - textSize.x) * .5f,
							min.y + ImGui::GetStyle().FramePadding.y);
	drawing->AddText(headerTextPos, textColor, manoeuvreTypeString);
	ImGui::PopFont();
	
	 
	glm::vec2 imageSize(headerStripWidth * 0.7);
	ImGui::SetCursorPosY(cursor.y + textSize.y + ImGui::GetStyle().ItemSpacing.y);
	ImGui::SetCursorPosX((headerStripWidth - imageSize.x) / 2.0);
	
	
	if(canStop()){
		ImGui::SetItemAllowOverlap();
		if(buttonStop("##stopManoeuvre", imageSize.x)) stop();
	}
	else{
		glm::vec2 imageUV1(.0f, .0f);
		glm::vec2 imageUV2(1.f, 1.f);
		glm::vec4 imageTint;
		if(b_selected) imageTint = glm::vec4(1.f, 1.f, 1.f, 1.f);
		else imageTint = glm::vec4(0.f, 0.f, 0.f, 1.f);
		glm::vec2 imagePos(min.x + (headerStripWidth - imageSize.x) * .5f, min.y + textSize.y);
		ImGui::SetItemAllowOverlap();
		ImGui::Image(icon->getID(), imageSize, imageUV1, imageUV2, imageTint);
	}
		
	
	
	//show name and description
	ImGui::SetCursorPosX(minCursor.x + headerStripWidth + ImGui::GetStyle().FramePadding.x);
	ImGui::SetCursorPosY(minCursor.y + ImGui::GetStyle().FramePadding.y);
	ImGui::PushFont(Fonts::sansBold15);
	float headerTextWidth = ImGui::GetContentRegionAvail().x;
	if(ImGui::CalcTextSize(getName()).x > headerTextWidth) scrollingText("ScrollingName", getName(), headerTextWidth, !b_hovered && !b_selected, 0.5);
	else ImGui::Text("%s", getName());
	ImGui::PopFont();
	ImGui::SetCursorPosY(minCursor.y + ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX(minCursor.x + headerStripWidth + ImGui::GetStyle().FramePadding.x);
	glm::vec2 descriptionCursor = ImGui::GetCursorPos();
	ImGui::PushFont(Fonts::sansLight15);
	ImGui::TextWrapped("%s", getDescription());
	ImGui::PopFont();
	
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
	
	
	
	
	//show playback or rapid state
	
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
	
	//mouse interaction and selection display
	if(b_hovered && !ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
		glm::vec4 color;
		if(ImGui::IsMouseDown(ImGuiMouseButton_Left)) color = glm::vec4(0.f, 0.f, 0.f, .3f);
		else color = glm::vec4(1.f, 1.f, 1.f, .1f);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(color), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	}
	if(b_selected){
		float thickness = ImGui::GetTextLineHeight() * 0.1;
		float rounding = ImGui::GetStyle().FrameRounding - thickness / 2.0;
		glm::vec2 minSelection = min + glm::vec2(thickness * .5f);
		glm::vec2 maxSelection = max - glm::vec2(thickness * .5f);
		ImGui::GetWindowDrawList()->AddRect(minSelection, maxSelection, ImColor(1.0f, 1.0f, 1.0f, .5f), rounding, ImDrawFlags_RoundCornersAll, thickness);
	}
	
}

float smoothstep(float edge0, float edge1, float x) {
	// Scale, bias and saturate x to 0..1 range
	x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

void Manoeuvre::miniatureGui(glm::vec2 size_arg){
	
	glm::vec4 backgroundColor;
	if(!b_valid){
		bool blink = (int)Timing::getProgramTime_milliseconds() % 1000 > 500;
		backgroundColor = blink ? Colors::yellow : Colors::red;
	}
	else if(areNoMachinesEnabled()) backgroundColor = Colors::red;
	else if(areAllMachinesEnabled()) backgroundColor = Colors::green;
	else backgroundColor = Colors::yellow;
	
	
	ImGui::PushFont(Fonts::sansBold15);
	scrollingTextWithBackground("ManoeuvreName", getName(), size_arg, backgroundColor);
	ImGui::PopFont();
	

	if(hasActiveAnimations()){
		auto activeAnimations = getActiveAnimations();
		int animationCount = activeAnimations.size();
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 size = ImGui::GetItemRectSize();
		float animationHeight = size.y / (float)animationCount;
		for(int i = 0; i < animationCount; i++){
			auto& animation = activeAnimations[i];
			float progress;
			if(animation->isInRapid()) progress = animation->getRapidProgress();
			else progress = animation->getPlaybackProgress();
			glm::vec2 minBar(min.x, min.y + animationHeight * i);
			glm::vec2 maxBar(minBar.x + size.x * progress, minBar.y + animationHeight);
			ImGui::GetWindowDrawList()->AddRectFilled(minBar, maxBar, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.1)), 5.0);
			
		}
	}
	
	if(ImGui::IsItemHovered()){
		ImGui::BeginTooltip();
		if(!b_valid) {
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
			ImGui::Text("Manoeuvre has Validation Errors");
			ImGui::PopStyleColor();
		}
		else ImGui::Text("Manoeuvre is Valid");
		if(!areAllMachinesEnabled()){
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
			for(auto& animation : animations){
				
				//if(!machine->isEnabled()) ImGui::Text("%s is not enabled.", machine->getName());
			}
			ImGui::PopStyleColor();
		}else ImGui::Text("All Machines are Enabled");
		ImGui::EndTooltip();
	}
}

void Manoeuvre::playbackGui(float height){
	
}


void Manoeuvre::sheetEditor(){
	
	
	auto currentProject = Stacato::Workspace::getCurrentProject();
	
	if(!currentProject->isPlotEditLocked()){
		
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		ImDrawListSplitter layers;
		layers.Split(drawing, 2);
		
		layers.SetCurrentChannel(drawing, 1);
		
		glm::vec2 titleMin, titleMax, descriptionMax;
		
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().FramePadding.x);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y);
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("%s", getName());
		ImGui::PopFont();
		
		titleMin = ImGui::GetItemRectMin();
		titleMax = ImGui::GetItemRectMax();
		
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().FramePadding.x);
		
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::TextWrapped("%s", getDescription());
		ImGui::PopStyleColor();
		ImGui::PopFont();
		
		descriptionMax = ImGui::GetItemRectMax();
		
		layers.SetCurrentChannel(drawing, 0);
		
		glm::vec2 padding = ImGui::GetStyle().FramePadding;
		float maxX = std::max(descriptionMax.x, titleMax.x);
		drawing->AddRectFilled(titleMin - padding, glm::vec2(maxX, descriptionMax.y) + padding, ImColor(Colors::almostBlack), ImGui::GetStyle().FrameRounding);
		drawing->AddRectFilled(titleMin - padding, glm::vec2(maxX, titleMax.y) + padding, ImColor(Colors::veryDarkGray), ImGui::GetStyle().FrameRounding);
		
		layers.Merge(drawing);
		
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y);
		
	}else{
	
		ImVec2 cursorPos = ImGui::GetCursorPos();
		
		ImGui::BeginGroup();
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Manoeuvre Type");
		ImGui::PopFont();
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 8.0);
		type->gui();
		 
		ImGui::EndGroup();
		
		cursorPos.x += ImGui::GetItemRectSize().x + ImGui::GetStyle().ItemSpacing.x;
		ImGui::SetCursorPos(cursorPos);
		
		ImGui::BeginGroup();
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Name");
		ImGui::PopFont();
		ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 15.0);
		name->gui();
		ImGui::EndGroup();
		
		cursorPos.x += ImGui::GetItemRectSize().x + ImGui::GetStyle().ItemSpacing.x;
		ImGui::SetCursorPos(cursorPos);
		
		ImGui::BeginGroup();
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Description");
		ImGui::PopFont();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		description->gui();
		ImGui::EndGroup();
			
	}
	
	int removedTrackIndex = -1;
	int movedUpTrackIndex = -1;
	int movedDownTrackIndex = -1;
	
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, glm::vec2(ImGui::GetTextLineHeight() * 0.15));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.15));
	ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX;
	
	if(Animation::beginTrackSheetTable(getType(), tableFlags)){
		ImGui::TableHeadersRow();
		
		for (int i = 0; i < animations.size(); i++) {
			ImGui::PushID(i);
			auto& animation = animations[i];
			
			//child parameter tracks are listed in the manoeuvres track vector
			//but they are drawn by the parent parameter group, so we skip them here
			//and draw them after the parameter group
			if (animation->hasParentComposite()) continue;
			
			ImGui::TableNextRow();
			
			
			if(currentProject->isPlotEditLocked()){
				
				//[1] "Playback"
				ImGui::TableSetColumnIndex(0);
				animation->playbackGui();
				
			}else{
				ImGui::TableSetColumnIndex(0);
				
				ListManagerWidget::Interaction interaction = ListManagerWidget::draw(animation == animations.front(), animation == animations.back(), "", ImGui::GetFrameHeight());
				switch(interaction){
					case ListManagerWidget::Interaction::NONE: break;
					case ListManagerWidget::Interaction::MOVE_UP: movedUpTrackIndex = i; break;
					case ListManagerWidget::Interaction::MOVE_DOWN: movedDownTrackIndex = i; break;
					case ListManagerWidget::Interaction::DELETE: removedTrackIndex = i; break;
				}
				
				//[1] "Playback"
				ImGui::TableSetColumnIndex(1);
				animation->playbackGui();
			}
			
		
			
			animation->baseTrackSheetRowGui();
			animation->trackSheetRowGui();
			
			//draw the groups child parameter tracks
			if(animation->isComposite()){
				auto composite = animation->toComposite();
				std::vector<std::shared_ptr<Animation>>& childAnimations = composite->getChildren();
				for(int j = 0; j < childAnimations.size(); j++){
					ImGui::TableNextRow();
					ImGui::PushID(j);
					ImGui::BeginDisabled(animation->isPlaying() || animation->isPaused());
					childAnimations[j]->baseTrackSheetRowGui();
					childAnimations[j]->trackSheetRowGui();
					ImGui::EndDisabled();
					ImGui::PopID();
				}
			}
			
			ImGui::PopID();
		}
		
		if(!currentProject->isPlotEditLocked()){
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			glm::vec2 addTrackButtonSize(ImGui::GetFrameHeight() * 3.0, ImGui::GetFrameHeight());
			if (ImGui::Button("Add Track", addTrackButtonSize)) ImGui::OpenPopup("ManoeuvreTrackAdder");
			if (ImGui::BeginPopup("ManoeuvreTrackAdder")) {
				ImGui::BeginDisabled();
				ImGui::MenuItem("Add Parameter Track");
				ImGui::MenuItem("Machine List :");
				ImGui::EndDisabled();
				ImGui::Separator();
				for (auto& machine : Environnement::getMachines()) {
					if(machine->getAnimatables().empty()) continue;
					
					if(machine->getAnimatables().size() == 1){
						auto animatable = machine->getAnimatables().front();
						if (animatable->hasParentComposite()) continue;
						bool isSelected = hasAnimation(animatable);
						if(ImGui::MenuItem(machine->getName(), nullptr, isSelected)){
							if(!isSelected) addAnimation(animatable);
							else removeAnimation(animatable);
						}
					}else if (ImGui::BeginMenu(machine->getName())) {
						for (auto& animatable : machine->getAnimatables()) {
							if (animatable->hasParentComposite()) continue;
							bool isSelected = hasAnimation(animatable);
							if (ImGui::MenuItem(animatable->getName(), nullptr, isSelected)) {
								if (!isSelected) addAnimation(animatable);
								else removeAnimation(animatable);
							}
						}
						ImGui::EndMenu();
					}

				}
				ImGui::EndPopup();
			}
		}
		
		if(animations.empty()){
			ImGui::TableNextColumn();
			ImGui::Text("No Tracks");
		}
		
		ImGui::EndTable();
	}
	
	ImGui::PopStyleVar(2);
	
	
	
	if(removedTrackIndex > -1) removeAnimation(getAnimations()[removedTrackIndex]->getAnimatable());
	if(movedUpTrackIndex > -1) moveAnimation(movedUpTrackIndex, movedUpTrackIndex - 1);
	if(movedDownTrackIndex > -1) moveAnimation(movedDownTrackIndex, movedDownTrackIndex + 1);
}




void Manoeuvre::spatialEditor(){
	ImGui::Text("Spatial Editor is not implemented yet.");
	ImGui::Text("We need a global 3D scene graph for this");
}











