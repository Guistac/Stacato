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



//inside draggable list element: InvisibleButton/ClipRectangle
void Manoeuvre::listGui(){
	
	//get drawing coordinates
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	glm::vec2 size = ImGui::GetItemRectSize();
	glm::vec2 minCursor = ImGui::GetCursorPos();
	bool b_hovered = ImGui::IsItemHovered();
	bool b_selected = isSelected();
	
	//draw background and show validness
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(Colors::veryDarkGray), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	if(!b_valid){
		bool blink = (int)Timing::getProgramTime_milliseconds() % 1000 > 500;
		glm::vec4 color = blink ? Colors::red : Colors::yellow;
		color.w = 0.5;
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(color), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
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
	ImGui::GetWindowDrawList()->AddRectFilled(min, maxHeaderStrip, ImColor(headerStripColor), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft);
	ImGui::PushFont(Fonts::sansRegular20);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.0f, 0.f, 0.f, .9f));
	glm::vec2 textSize = ImGui::CalcTextSize(manoeuvreTypeString);
	ImGui::SetCursorPosX(minCursor.x + (headerStripWidth - textSize.x) / 2.0);
	ImGui::SetCursorPosY(minCursor.y + ImGui::GetStyle().FramePadding.y);
	ImGui::Text("%s", manoeuvreTypeString);
	ImGui::PopFont();
	ImGui::PopStyleColor();
	glm::vec2 imageSize(headerStripWidth * 0.7);
	glm::vec4 imageTint(1.f, 1.f, 1.f,.9f);
	glm::vec2 imageUV1(.0f, .0f);
	glm::vec2 imageUV2(1.f, 1.f);
	float posY = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(posY - ImGui::GetStyle().ItemSpacing.y);
	ImGui::SetCursorPosX((headerStripWidth - imageSize.x) / 2.0);
	ImGui::Image(icon->getID(), imageSize, imageUV1, imageUV2, imageTint);
	
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
	ImGui::PushFont(Fonts::sansLight15);
	ImGui::TextWrapped("%s", getDescription());
	ImGui::PopFont();
	
	//show playback or rapid state
	
	if(hasActiveAnimations()){
		auto activeAnimations = getActiveAnimations();
		int animationCount = activeAnimations.size();
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
		glm::vec2 minSelection = min + glm::vec2(thickness / 2.0);
		glm::vec2 maxSelection = max - glm::vec2(thickness / 2.0);
		ImGui::GetWindowDrawList()->AddRect(minSelection, maxSelection, ImColor(1.0f, 1.0f, 1.0f, 0.3f), rounding, ImDrawFlags_RoundCornersAll, thickness);
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


void Manoeuvre::sheetEditor(){
	
	
	
	
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
		 
	ImGui::Separator();
	
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
				if (ImGui::BeginMenu(machine->getName())) {
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

void Manoeuvre::curveEditor(){
	
				
	if (ImGui::Button("Center On Curves") || shouldRefocusCurves()) {
		double minX, maxX, minY, maxY;
		getCurveRange(minX, maxX, minY, maxY);
		double rangeX = maxX - minX;
		double rangeY = maxY - minY;
		double extraRange = 0.05;
		minX -= rangeX * extraRange;
		maxX += rangeX * extraRange;
		minY -= rangeY * extraRange;
		maxY += rangeY * extraRange;
		ImPlot::SetNextPlotLimits(minX, maxX, minY, maxY, ImGuiCond_Always);
	}
	
	ImPlotFlags plotFlags = ImPlotFlags_AntiAliased | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoChild;
	
	if (ImPlot::BeginPlot("##SequenceCurveDisplay", 0, 0, ImGui::GetContentRegionAvail(), plotFlags)) {
		
		if (getType() != ManoeuvreType::KEY) {
			//draw manoeuvre bounds
			glm::vec2 plotBoundsMin(ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().Y.Max);
			glm::vec2 plotBoundsMax(ImPlot::GetPlotLimits().X.Max, ImPlot::GetPlotLimits().Y.Min);
			double startTime = 0.0;
			double endTime = getDuration();
			std::vector<glm::vec2> limits;
			limits.push_back(glm::vec2(plotBoundsMin.x, plotBoundsMin.y));
			limits.push_back(glm::vec2(startTime, plotBoundsMin.y));
			limits.push_back(glm::vec2(startTime, plotBoundsMax.y));
			limits.push_back(glm::vec2(endTime, plotBoundsMax.y));
			limits.push_back(glm::vec2(endTime, plotBoundsMin.y));
			limits.push_back(glm::vec2(plotBoundsMax.x, plotBoundsMin.y));
			if (endTime > 0.0) {
				ImPlot::SetNextFillStyle(Colors::black, 0.5);
				ImPlot::PlotShaded("##shaded", &limits.front().x, &limits.front().y, limits.size(), -INFINITY, 0, sizeof(glm::vec2));
				ImPlot::PlotVLines("##Limits1", &startTime, 1);
				ImPlot::PlotVLines("##Limits2", &endTime, 1);
			}
		}
		
		for (auto& animation : getAnimations()) animation->drawCurves();
		for (auto& animation : getAnimations()) {
			ImGui::PushID(animation->getAnimatable()->getMachine()->getName());
			ImGui::PushID(animation->getAnimatable()->getName());
			animation->drawCurveControls();
			ImGui::PopID();
			ImGui::PopID();
		}
		
		if(getType() != ManoeuvreType::KEY){
			double playbackTime = getSychronizedPlaybackPosition();
			if(!isnan(playbackTime)){
				ImPlot::SetNextLineStyle(Colors::white, ImGui::GetTextLineHeight() * 0.1);
				ImPlot::PlotVLines("Playhead", &playbackTime, 1);
			}
		}
		
		if(hasActiveAnimations()){
			for(auto& animation : getAnimations()){
				if(!animation->isActive()) continue;
				auto animatable = animation->getAnimatable();
				double playbackTime = animation->getPlaybackPosition();
				auto actualValue = animatable->getActualValue();
				auto curveValues = animatable->getCurvePositionsFromAnimationValue(actualValue);
				int curveCount = animatable->getCurveCount();
				std::vector<glm::vec2> playbackIndicators(curveCount);
				for(int i = 0; i < curveCount; i++){
					playbackIndicators[i].x = playbackTime;
					playbackIndicators[i].y = curveValues[i];
				}
				ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, ImGui::GetTextLineHeight() * .2f, Colors::white, -1, ImVec4(0.0, 0.0, 0.0, 0.0));
				ImPlot::PlotScatter("##PlayackIndicator", &playbackIndicators.front().x, &playbackIndicators.front().y, curveCount);
			}
		}
		

		ImPlot::EndPlot();
	}
	
}




void Manoeuvre::spatialEditor(){
	ImGui::Text("Spatial Editor is not implemented yet.");
	ImGui::Text("We need a global 3D scene graph for this");
}











