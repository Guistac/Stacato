#include <pch.h>

#include "Motion/Manoeuvre/Manoeuvre.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"
#include "Machine/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Motion/Curve/Curve.h"
#include "Plot/Plot.h"

#include "Motion/Playback/Playback.h"

#include "Gui/Utilities/CustomWidgets.h"

void Manoeuvre::listGui(){
	//inside draggable list element (BeginChild)
	
	//show manoeuvre validness
	if(!b_valid){
		glm::vec2 min = ImGui::GetWindowPos();
		glm::vec2 max = min + glm::vec2(ImGui::GetWindowSize());
		bool blink = (int)Timing::getProgramTime_milliseconds() % 1000 > 500;
		glm::vec4 color = blink ? Colors::red : Colors::yellow;
		color.w = 0.5;
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(color), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	}
	
	//show header type strip
	float headerStripWidth = 50.0;
	glm::vec2 min = ImGui::GetWindowPos();
	glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
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
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(headerStripColor), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft);
	
	
	const char* manoeuvreTypeString;
	switch(getType()){
		case ManoeuvreType::KEY: manoeuvreTypeString = "KEY"; break;
		case ManoeuvreType::TARGET: manoeuvreTypeString = "TAR"; break;
		case ManoeuvreType::SEQUENCE: manoeuvreTypeString = "SEQ"; break;
	}
	ImGui::PushFont(Fonts::sansRegular20);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.0f, 0.f, 0.f, .9f));
	glm::vec2 textSize = ImGui::CalcTextSize(manoeuvreTypeString);
	ImGui::SetCursorPosX((headerStripWidth - textSize.x) / 2.0);
	ImGui::SetCursorPosY(ImGui::GetStyle().FramePadding.y);
	ImGui::Text("%s", manoeuvreTypeString);
	ImGui::PopFont();
	ImGui::PopStyleColor();
	
	
	//show name and description
	ImGui::SetCursorPosX(headerStripWidth + ImGui::GetStyle().FramePadding.x);
	ImGui::SetCursorPosY(ImGui::GetStyle().FramePadding.y);
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("%s", getName());
	ImGui::PopFont();
	
	ImGui::SetCursorPosY(ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX(headerStripWidth + ImGui::GetStyle().FramePadding.x);
	
	ImGui::PushFont(Fonts::sansLight15);
	ImGui::TextWrapped("%s", getDescription());
	ImGui::PopFont();
	
	if(isInRapid()){
		glm::vec2 windowPos = ImGui::GetWindowPos();
		glm::vec2 maxsize = ImGui::GetWindowSize();
		int trackCount = tracks.size();
		float trackHeight = maxsize.y / (float)trackCount;
		for (int i = 0; i < trackCount; i++) {
			glm::vec2 min(windowPos.x, windowPos.y + trackHeight * i);
			glm::vec2 max(min.x + maxsize.x * tracks[i]->getRapidProgress(), min.y + trackHeight);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.1)), 5.0);
		}
	}else if(isPlaying() || isPaused()){
		glm::vec2 min = ImGui::GetWindowPos();
		glm::vec2 windowSize = ImGui::GetWindowSize();
		float progress = getPlaybackProgress();
		glm::vec2 max(min.x + windowSize.x * progress, min.y + windowSize.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.4)), 5.0);
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
			for(auto& track : tracks){
				auto machine = track->getParameter()->getMachine();
				if(!machine->isEnabled()) ImGui::Text("%s is not enabled.", machine->getName());
			}
			ImGui::PopStyleColor();
		}else ImGui::Text("All Machines are Enabled");
		ImGui::EndTooltip();
	}
	
	if(isInRapid()){
		glm::vec2 pos = ImGui::GetItemRectMin();
		glm::vec2 size = ImGui::GetItemRectSize();
		int trackCount = tracks.size();
		float trackHeight = size.y / (float)trackCount;
		for (int i = 0; i < trackCount; i++) {
			glm::vec2 min(pos.x, pos.y + trackHeight * i);
			glm::vec2 max(pos.x + size.x * tracks[i]->getRapidProgress(), pos.y + trackHeight);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.1)), 5.0);
		}
	}else if(isPlaying() || isPaused()){
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 size = ImGui::GetItemRectSize();
		float progress = getPlaybackProgress();
		glm::vec2 max(min.x + size.x * progress, min.y + size.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.4)), 5.0);
	}
}


void Manoeuvre::trackSheetGui(){
	
	ImVec2 cursorPos = ImGui::GetCursorPos();
	
	ImGui::BeginGroup();
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Manoeuvre Type");
	ImGui::PopFont();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 8.0);
	
	static ManoeuvreType availableManoeuvreTypes[2] = {
		ManoeuvreType::KEY,
		ManoeuvreType::SEQUENCE
	};
	
	type->combo(availableManoeuvreTypes, 2);
	
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
			b_tableBegun = ImGui::BeginTable("##TrackParameters", 9, tableFlags);
			ImGui::TableSetupColumn("Manage");
			ImGui::TableSetupColumn("Machine");
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Interpolation");	//kinematic, linear, step, bezier
			ImGui::TableSetupColumn("Target");			//position or other
			ImGui::TableSetupColumn("Using");			//time vs velocity
			ImGui::TableSetupColumn("Constraint");		//time or velocity
			ImGui::TableSetupColumn("Time Offset");		//seconds
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
		
		for (int i = 0; i < tracks.size(); i++) {
			ImGui::PushID(i);
			auto& parameterTrack = tracks[i];
			
			//child parameter tracks are listed in the manoeuvres track vector
			//but they are drawn by the parent parameter group, so we skip them here
			//and draw them after the parameter group
			if (parameterTrack->hasParentGroup()) continue;
			
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			
			ListManagerWidget::Interaction interaction = ListManagerWidget::draw(parameterTrack == tracks.front(), parameterTrack == tracks.back(), "", ImGui::GetFrameHeight());
			switch(interaction){
				case ListManagerWidget::Interaction::NONE: break;
				case ListManagerWidget::Interaction::MOVE_UP: movedUpTrackIndex = i; break;
				case ListManagerWidget::Interaction::MOVE_DOWN: movedDownTrackIndex = i; break;
				case ListManagerWidget::Interaction::DELETE: removedTrackIndex = i; break;
			}
			
			parameterTrack->baseTrackSheetRowGui();
			parameterTrack->trackSheetRowGui();
			
			//draw the groups child parameter tracks
			if(parameterTrack->isGroup()){
				auto groupTrack = parameterTrack->castToGroup();
				std::vector<std::shared_ptr<ParameterTrack>>& childTracks = groupTrack->getChildren();
				for(int j = 0; j < groupTrack->getChildren().size(); j++){
					ImGui::PushID(j);
					childTracks[j]->baseTrackSheetRowGui();
					childTracks[j]->trackSheetRowGui();
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
				if(machine->parameters.empty()) continue;
				if (ImGui::BeginMenu(machine->getName())) {
					for (auto& parameter : machine->parameters) {
						if (parameter->hasParentGroup()) continue;
						bool isSelected = hasTrack(parameter);
						if (ImGui::MenuItem(parameter->getName(), nullptr, isSelected)) {
							if (!isSelected) addTrack(parameter);
							else removeTrack(parameter);
						}
					}
					ImGui::EndMenu();
				}
			}
			ImGui::EndPopup();
		}
		if(tracks.empty()){
			ImGui::TableNextColumn();
			ImGui::Text("No Tracks");
		}
		
		ImGui::EndTable();
	}
	
	ImGui::PopStyleVar(2);
	
	if(removedTrackIndex > -1) removeTrack(getTracks()[removedTrackIndex]->getParameter());
	if(movedUpTrackIndex > -1) moveTrack(movedUpTrackIndex, movedUpTrackIndex - 1);
	if(movedDownTrackIndex > -1) moveTrack(movedDownTrackIndex, movedDownTrackIndex + 1);
}

void Manoeuvre::curveEditorGui(){
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
		
		for (auto& parameterTrack : getTracks()) parameterTrack->drawCurves();
		for (auto& parameterTrack : getTracks()) {
			ImGui::PushID(parameterTrack->getParameter()->getMachine()->getName());
			ImGui::PushID(parameterTrack->getParameter()->getName());
			parameterTrack->drawCurveControls();
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




void Manoeuvre::spatialEditorGui(){}











