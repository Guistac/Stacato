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

void Manoeuvre::listGui(){

	//inside draggable list element
	
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	glm::vec2 size = ImGui::GetItemRectSize();
	glm::vec2 minCursor = ImGui::GetCursorPos();
	bool b_hovered = ImGui::IsItemHovered();
	
	//show manoeuvre validness
	if(!b_valid){
		bool blink = (int)Timing::getProgramTime_milliseconds() % 1000 > 500;
		glm::vec4 color = blink ? Colors::red : Colors::yellow;
		color.w = 0.5;
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(color), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	}else{
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(Colors::veryDarkGray), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	}
	
	//show header type strip
	float headerStripWidth = 50.0;
	glm::vec2 maxHeaderStrip = min + glm::vec2(headerStripWidth, ImGui::GetItemRectSize().y);
	glm::vec4 headerStripColor;
	switch (type->value) {
		case ManoeuvreType::KEY:
			headerStripColor = Colors::darkYellow;
			break;
		case ManoeuvreType::TARGET:
			headerStripColor = Colors::darkGray;
			break;
		case ManoeuvreType::SEQUENCE:
			headerStripColor = Colors::darkRed;
			break;
	}
	ImGui::GetWindowDrawList()->AddRectFilled(min, maxHeaderStrip, ImColor(headerStripColor), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft);
	
	
	const char* manoeuvreTypeString;
	switch(getType()){
		case ManoeuvreType::KEY: manoeuvreTypeString = "KEY"; break;
		case ManoeuvreType::TARGET: manoeuvreTypeString = "TAR"; break;
		case ManoeuvreType::SEQUENCE: manoeuvreTypeString = "SEQ"; break;
	}
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
	
	
	switch(getType()){
		case ManoeuvreType::KEY:
			ImGui::Image(Images::KeyIcon.getID(), imageSize, imageUV1, imageUV2, imageTint); break;
		case ManoeuvreType::TARGET:
			ImGui::Image(Images::TargetIcon.getID(), imageSize, imageUV1, imageUV2, imageTint); break;
		case ManoeuvreType::SEQUENCE:
			ImGui::Image(Images::SequenceIcon.getID(), imageSize, imageUV1, imageUV2, imageTint); break;
	}
	
	
	//show name and description
	ImGui::SetCursorPosX(minCursor.x + headerStripWidth + ImGui::GetStyle().FramePadding.x);
	ImGui::SetCursorPosY(minCursor.y + ImGui::GetStyle().FramePadding.y);
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("%s", getName());
	ImGui::PopFont();
	
	ImGui::SetCursorPosY(minCursor.y + ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX(minCursor.x + headerStripWidth + ImGui::GetStyle().FramePadding.x);
	
	ImGui::PushFont(Fonts::sansLight15);
	ImGui::TextWrapped("%s", getDescription());
	ImGui::PopFont();
	
	if(isInRapid()){
		int trackCount = animations.size();
		float trackHeight = size.y / (float)trackCount;
		for (int i = 0; i < trackCount; i++) {
			glm::vec2 minBar(min.x, min.y + trackHeight * i);
			glm::vec2 maxBar(min.x + size.x * animations[i]->getRapidProgress(), min.y + trackHeight);
			ImGui::GetWindowDrawList()->AddRectFilled(min, maxBar, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.1)), 5.0);
		}
	}else if(isPlaying() || isPaused()){
		float progress = getPlaybackProgress();
		glm::vec2 maxBar(min.x + size.x * progress, min.y + size.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, maxBar, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.4)), 5.0);
	}
	
	if(b_hovered && !ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
		glm::vec4 color;
		if(ImGui::IsMouseDown(ImGuiMouseButton_Left)) color = glm::vec4(0.f, 0.f, 0.f, .3f);
		else color = glm::vec4(1.f, 1.f, 1.f, .1f);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(color), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	}
	
	if(isSelected()){
		float thickness = ImGui::GetTextLineHeight() * 0.1;
		float rounding = ImGui::GetStyle().FrameRounding - thickness / 2.0;
		glm::vec2 minSelection = min + glm::vec2(thickness / 2.0);
		glm::vec2 maxSelection = max - glm::vec2(thickness / 2.0);
		ImGui::GetWindowDrawList()->AddRect(minSelection, maxSelection, ImColor(1.0f, 1.0f, 1.0f, 0.3f), rounding, ImDrawFlags_RoundCornersAll, thickness);
	}
	
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
	backgroundText(getName(), size_arg, backgroundColor);
	
	if(isInRapid()){
		glm::vec2 pos = ImGui::GetItemRectMin();
		glm::vec2 size = ImGui::GetItemRectSize();
		int trackCount = animations.size();
		float trackHeight = size.y / (float)trackCount;
		for (int i = 0; i < trackCount; i++) {
			glm::vec2 min(pos.x, pos.y + trackHeight * i);
			glm::vec2 max(pos.x + size.x * animations[i]->getRapidProgress(), pos.y + trackHeight);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.1)), 5.0);
		}
	}else if(isPlaying() || isPaused()){
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 size = ImGui::GetItemRectSize();
		float progress = getPlaybackProgress();
		glm::vec2 max(min.x + size.x * progress, min.y + size.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.4)), 5.0);
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
				auto machine = animation->getAnimatable()->getMachine();
				if(!machine->isEnabled()) ImGui::Text("%s is not enabled.", machine->getName());
			}
			ImGui::PopStyleColor();
		}else ImGui::Text("All Machines are Enabled");
		ImGui::EndTooltip();
	}
}


void Manoeuvre::sheetEditor(){
	
	ImGui::BeginDisabled(isPlaying() || isPaused());
	
	ImVec2 cursorPos = ImGui::GetCursorPos();
	
	ImGui::BeginGroup();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Manoeuvre Type");
	ImGui::PopFont();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 8.0);
	
	/*
	static ManoeuvreType availableManoeuvreTypes[2] = {
		ManoeuvreType::KEY,
		ManoeuvreType::SEQUENCE
	};
	type->combo(availableManoeuvreTypes, 2);
	*/
	type->gui();
	 
	//type->gui();
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
	
	bool b_tableBegun;
	
	switch(getType()){
		case ManoeuvreType::KEY:
			b_tableBegun = ImGui::BeginTable("##TrackParameters", 4, tableFlags);
			ImGui::TableSetupColumn("Manage");
			ImGui::TableSetupColumn("Machine");
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Target");
			break;
		case ManoeuvreType::TARGET:
			b_tableBegun = ImGui::BeginTable("##TrackParameters", 8, tableFlags);
			ImGui::TableSetupColumn("Manage");
			ImGui::TableSetupColumn("Machine");
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Interpolation");	//kinematic, linear, step, bezier
			ImGui::TableSetupColumn("Target");			//position or other
			ImGui::TableSetupColumn("Using");			//time vs velocity
			ImGui::TableSetupColumn("Constraint");		//time or velocity
			ImGui::TableSetupColumn("Ramps");			//for kinematic or bezier
			break;
		case ManoeuvreType::SEQUENCE:
			b_tableBegun = ImGui::BeginTable("##TrackParameters", 9, tableFlags);
			ImGui::TableSetupColumn("Manage");
			ImGui::TableSetupColumn("Machine");
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Interpolation");
			ImGui::TableSetupColumn("Start");		//sequencer start
			ImGui::TableSetupColumn("End");			//sequencer end
			ImGui::TableSetupColumn("Duration");
			ImGui::TableSetupColumn("Time Offset");
			ImGui::TableSetupColumn("Ramps");
			break;
	}
	
	if(b_tableBegun){
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
			
			animation->baseTrackSheetRowGui();
			animation->trackSheetRowGui();
			
			//draw the groups child parameter tracks
			if(animation->isComposite()){
				auto composite = animation->toComposite();
				std::vector<std::shared_ptr<Animation>>& childAnimations = composite->getChildren();
				for(int j = 0; j < childAnimations.size(); j++){
					ImGui::TableNextRow();
					ImGui::PushID(j);
					childAnimations[j]->baseTrackSheetRowGui();
					childAnimations[j]->trackSheetRowGui();
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
				if(machine->animatables.empty()) continue;
				if (ImGui::BeginMenu(machine->getName())) {
					for (auto& animatable : machine->animatables) {
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
	
	ImGui::EndDisabled();
	
	if(removedTrackIndex > -1) removeAnimation(getAnimations()[removedTrackIndex]->getAnimatable());
	if(movedUpTrackIndex > -1) moveAnimation(movedUpTrackIndex, movedUpTrackIndex - 1);
	if(movedDownTrackIndex > -1) moveAnimation(movedDownTrackIndex, movedDownTrackIndex + 1);
}

void Manoeuvre::curveEditor(){
	if (ImGui::Button("Center On Curves")) ImPlot::FitNextPlotAxes();
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
			double playbackTime = getPlaybackPosition();
			ImPlot::SetNextLineStyle(Colors::white, ImGui::GetTextLineHeight() * 0.1);
			ImPlot::PlotVLines("Playhead", &playbackTime, 1);
		}
		

		ImPlot::EndPlot();
	}
	
}




void Manoeuvre::spatialEditor(){}











