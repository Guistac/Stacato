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
	
	/*
	if (Playback::isInRapid(manoeuvre)) {
		glm::vec2 windowPos = ImGui::GetWindowPos();
		glm::vec2 maxsize = ImGui::GetWindowSize();
		int trackCount = manoeuvre->tracks.size();
		float trackHeight = maxsize.y / (float)trackCount;
		for (int i = 0; i < trackCount; i++) {
			glm::vec2 min(windowPos.x, windowPos.y + trackHeight * i);
			glm::vec2 max(min.x + maxsize.x * manoeuvre->tracks[i]->getRapidProgress(), min.y + trackHeight);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.1)), 5.0);
		}
	}

	if (Playback::isPlaying(manoeuvre)) {
		glm::vec2 min = ImGui::GetWindowPos();
		glm::vec2 windowSize = ImGui::GetWindowSize();
		float progress = manoeuvre->getPlaybackProgress();
		glm::vec2 max(min.x + windowSize.x * progress, min.y + windowSize.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.4)), 5.0);
	}
	 */
	
}

void Manoeuvre::miniatureGui(glm::vec2 size_arg){
	
	glm::vec4 backgroundColor = Colors::darkGreen;
	if(!b_valid){
		bool blink = (int)Timing::getProgramTime_milliseconds() % 1000 > 500;
		backgroundColor = blink ? Colors::red : Colors::yellow;
	}
	backgroundText(getName(), size_arg, backgroundColor);
}


void Manoeuvre::trackSheetGui(){

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
			b_tableBegun = ImGui::BeginTable("##TrackParameters", 7, tableFlags);
			ImGui::TableSetupColumn("Manage");
			ImGui::TableSetupColumn("Machine");
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Start");		//sequencer start
			ImGui::TableSetupColumn("End");			//sequencer end
			ImGui::TableSetupColumn("Duration");
			ImGui::TableSetupColumn("Time Offset");
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

void Manoeuvre::curveEditorGui(){}

void Manoeuvre::spatialEditorGui(){}












/*
void Manoeuvre::curveEditorGui(const std::shared_ptr<Manoeuvre>& manoeuvre){
    if (ImGui::Button("Center On Curves")) ImPlot::FitNextPlotAxes();
    ImPlotFlags plotFlags = ImPlotFlags_AntiAliased | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoChild;
    if (ImPlot::BeginPlot("##SequenceCurveDisplay", 0, 0, ImGui::GetContentRegionAvail(), plotFlags)) {
        
        for (auto& parameterTrack : manoeuvre->tracks) parameterTrack->drawChainedCurves();
        
        if (manoeuvre->type != Manoeuvre::Type::KEY_POSITION) {
            //draw manoeuvre bounds
            glm::vec2 plotBoundsMin(ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().Y.Max);
            glm::vec2 plotBoundsMax(ImPlot::GetPlotLimits().X.Max, ImPlot::GetPlotLimits().Y.Min);
            double startTime = 0.0;
            double endTime = manoeuvre->getLength_seconds();
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
            for (auto& parameterTrack : manoeuvre->tracks) parameterTrack->drawCurves(startTime, endTime);
            for (auto& parameterTrack : manoeuvre->tracks) {
                ImGui::PushID(parameterTrack->parameter->machine->getName());
                ImGui::PushID(parameterTrack->parameter->name);
                bool controlPointEdited = parameterTrack->drawControlPoints();
                ImGui::PopID();
                ImGui::PopID();
            }
            double playbackTime = manoeuvre->playbackPosition_seconds;
            ImPlot::SetNextLineStyle(Colors::white, ImGui::GetTextLineHeight() * 0.1);
            ImPlot::PlotVLines("Playhead", &playbackTime, 1);
        }
        else {
            double zero = 0.0;
            ImPlot::SetNextLineStyle(Colors::white, 2.0);
            ImPlot::PlotVLines("##ZeroTime", &zero, 1);
            for (auto& parameterTrack : manoeuvre->tracks) {
                ImGui::PushID(parameterTrack->parameter->machine->getName());
                ImGui::PushID(parameterTrack->parameter->name);
                bool controlPointEdited = parameterTrack->drawControlPoints();
                ImGui::PopID();
                ImGui::PopID();
            }
        }

        ImPlot::EndPlot();
    }
}
 */

/*
void Manoeuvre::spatialEditorGui(const std::shared_ptr<Manoeuvre>& manoeuvre){
    //ImGui::Text("2D or 3D view of trajectory editor with timeline");
}
 */




/*
void Manoeuvre::playbackControlGui(const std::shared_ptr<Manoeuvre>& manoeuvre) {

	static auto fastMoveProgressOverlay = [&]() {
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 size = ImGui::GetItemRectSize();
		glm::vec2 max(min.x + size.x * Playback::getRapidProgress(manoeuvre), min.y + size.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
	};
	static auto playbackProgressOverlay = [&]() {
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 size = ImGui::GetItemRectSize();
		glm::vec2 max(min.x + size.x * manoeuvre->getPlaybackProgress(), min.y + size.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.4)), 5.0);
	};

	float buttonHeight = ImGui::GetTextLineHeight() * 2.0;
	float availableWidth = ImGui::GetContentRegionAvail().x;
	glm::vec2 singleButtonSize(availableWidth, buttonHeight);
	glm::vec2 doubleButtonSize((availableWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, buttonHeight);
	glm::vec2 tripleButtonSize((availableWidth - ImGui::GetStyle().ItemSpacing.x * 2.0) / 3.0, buttonHeight);


	switch (manoeuvre->type) {
		case Manoeuvre::Type::KEY_POSITION:



			if (Playback::isInRapid(manoeuvre)) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Cancel Rapid", singleButtonSize)) Playback::stopRapid(manoeuvre);
				ImGui::PopStyleColor();
				fastMoveProgressOverlay();
			}
			else {
				bool isAtKeyPosition = Playback::isPrimedToEnd(manoeuvre);
				if (isAtKeyPosition) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					ImGui::Button("At Key Position", singleButtonSize);
					ImGui::PopStyleColor();
					ImGui::PopItemFlag();
				}
				else {
					if (ImGui::Button("Rapid To Key Position", singleButtonSize)) Playback::rapidToEnd(manoeuvre);
				}
			}
			break;



		case Manoeuvre::Type::TIMED_MOVEMENT:


			if (Playback::isInRapid(manoeuvre)) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Cancel Rapid", singleButtonSize)) Playback::stopRapid(manoeuvre);
				ImGui::PopStyleColor();
				fastMoveProgressOverlay();
			}
			else {
				bool disableRapidButtons = Playback::isPlaying(manoeuvre);
				if (disableRapidButtons) BEGIN_DISABLE_IMGUI_ELEMENT
				bool primedToStart = Playback::isPrimedToStart(manoeuvre);
				if (primedToStart) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				}
				if (ImGui::Button("Rapid To Start", doubleButtonSize)) Playback::rapidToStart(manoeuvre);
				if (primedToStart) {
					ImGui::PopStyleColor();
					ImGui::PopItemFlag();
				}
				ImGui::SameLine();
				bool primedToEnd = Playback::isPrimedToEnd(manoeuvre);
				if (primedToEnd) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				}
				if (ImGui::Button("Rapid To End", doubleButtonSize)) Playback::rapidToEnd(manoeuvre);
				if (primedToEnd) {
					ImGui::PopStyleColor();
					ImGui::PopItemFlag();
				}
				if (disableRapidButtons) END_DISABLE_IMGUI_ELEMENT
			}
			if (Playback::isPlaying(manoeuvre)) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkRed);
				if (ImGui::Button("Stop", doubleButtonSize)) Playback::stopPlayback(manoeuvre);
				ImGui::PopStyleColor();
				ImGui::SameLine();
				if (Playback::isPaused(manoeuvre)) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("Resume", doubleButtonSize)) Playback::resumePlayback(manoeuvre);
					ImGui::PopStyleColor();
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("Pause", doubleButtonSize)) Playback::pausePlayback(manoeuvre);
					ImGui::PopStyleColor();
				}
				playbackProgressOverlay();
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				if (ImGui::Button("Timed Move", singleButtonSize)) Playback::startPlayback(manoeuvre);
				ImGui::PopStyleColor();
			}
			break;



		case Manoeuvre::Type::MOVEMENT_SEQUENCE:

			//=== RAPID BUTTONS ===
			if (Playback::isInRapid(manoeuvre)) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				if (ImGui::Button("Cancel Rapid", singleButtonSize)) Playback::stopRapid(manoeuvre);
				ImGui::PopStyleColor();
				fastMoveProgressOverlay();
			}
			else {
				bool disableRapidButtons = Playback::isPlaying(manoeuvre);
				ImGui::BeginDisabled(disableRapidButtons);
				
				bool primedToStart = Playback::isPrimedToStart(manoeuvre);
				if (primedToStart) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::Button("At Start", doubleButtonSize);
					ImGui::PopStyleColor();
					ImGui::PopItemFlag();
				}else if (ImGui::Button("Rapid To Start", doubleButtonSize)) Playback::rapidToStart(manoeuvre);
				
				ImGui::SameLine();
				bool primedToEnd = Playback::isPrimedToEnd(manoeuvre);
				if (primedToEnd) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::Button("At End", doubleButtonSize);
					ImGui::PopStyleColor();
					ImGui::PopItemFlag();
				}
				else if (ImGui::Button("Rapid To End", doubleButtonSize)) Playback::rapidToEnd(manoeuvre);
				ImGui::EndDisabled();
			}

			//=== PLAYBACK POSITION CONTROL ===
			ImGui::SetNextItemWidth(singleButtonSize.x);
			if (Playback::isPlaying(manoeuvre) && !Playback::isPaused(manoeuvre)) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::InputDouble("##playbackPosition", &manoeuvre->playbackPosition_seconds, 0.0, 0.0, "%.3f s");
				ImGui::PopItemFlag();
			}
			else {
				ImGui::InputDouble("##targetTime", &manoeuvre->playbackPosition_seconds, 0.0, 0.0, "Playback Position : %.1f seconds");
				manoeuvre->playbackPosition_seconds = std::max(0.0, manoeuvre->playbackPosition_seconds);
				manoeuvre->playbackPosition_seconds = std::min(manoeuvre->getLength_seconds(), manoeuvre->playbackPosition_seconds);
			}
			if(manoeuvre->getPlaybackProgress() != 0.0) playbackProgressOverlay();
			
			//=== PLAYBACK CONTROL BUTTON ===
			if (Playback::isPlaying(manoeuvre)) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkRed);
				//if (ImGui::Button("Stop", doubleButtonSize)) Playback::stopPlayback(manoeuvre);
				if (ImGui::Button("Stop", singleButtonSize)) Playback::stopPlayback(manoeuvre);
				ImGui::PopStyleColor();
				/*
				ImGui::SameLine();
				if (Playback::isPaused(manoeuvre)) {
					if (!Playback::isPrimedToPlaybackPosition(manoeuvre)) {
						ImGui::PushStyleColor(ImGuiCol_Button, Colors::orange);
						if (ImGui::Button("Rapid To Time", doubleButtonSize)) Playback::rapidToPlaybackPosition(manoeuvre);
						ImGui::PopStyleColor();
					}
					else {
						ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
						if (ImGui::Button("Resume", doubleButtonSize)) Playback::resumePlayback(manoeuvre);
						ImGui::PopStyleColor();
					}
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("Pause", doubleButtonSize)) Playback::pausePlayback(manoeuvre);
					ImGui::PopStyleColor();
				}
			}
			else {
				if (!Playback::isPrimedToPlaybackPosition(manoeuvre)) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::orange);
					if (ImGui::Button("Rapid to Playback Position", singleButtonSize)) Playback::rapidToPlaybackPosition(manoeuvre);
					ImGui::PopStyleColor();
				}
				bool disableStart = !Playback::isPrimedToStart(manoeuvre);
				if(disableStart) {
					backgroundText("Not At Sequence Start", singleButtonSize, Colors::gray);
				}else {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("Start Playback", singleButtonSize)) Playback::startPlayback(manoeuvre);
					ImGui::PopStyleColor();
				}

		

			}

		break;
	}
}
 */


/*
float Manoeuvre::getPlaybackControlGuiHeight(const std::shared_ptr<Manoeuvre>& manoeuvre) {
	switch (manoeuvre->type) {
		case Manoeuvre::Type::KEY_POSITION:
			return ImGui::GetTextLineHeight() * 2.0 //single row of buttons
				+ ImGui::GetStyle().ItemSpacing.y;	//spacing
		case Manoeuvre::Type::TIMED_MOVEMENT:
			return ImGui::GetTextLineHeight() * 2.0 * 2.0	//single row of buttons
				+ ImGui::GetStyle().ItemSpacing.y * 2.0;	//spacing
		case Manoeuvre::Type::MOVEMENT_SEQUENCE:
			return ImGui::GetTextLineHeight() * 2.0 * 2.0 //two rows of buttons
				+ ImGui::GetFrameHeight()			//one row of widgets
				+ImGui::GetStyle().ItemSpacing.y * 3.0;	//three spacings
			return ImGui::GetTextLineHeight() * 2.0 * 2.0 //two rows of buttons
				+ImGui::GetStyle().ItemSpacing.y * 2.0;	//two spacings
		default: return 0.0;
	}
}
*/
