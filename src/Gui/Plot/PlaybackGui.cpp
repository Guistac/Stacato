#include <pch.h>
#include "PlaybackGui.h"

#include <imgui.h>

#include "Project/Project.h"
#include "Plot/Plot.h"
#include "Plot/ManoeuvreList.h"
#include "Motion/Manoeuvre/Manoeuvre.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Motion/Playback/Playback.h"

#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Machine/Machine.h"

namespace Playback::Gui{

void manoeuvrePlaybackControls(float height){
	
	std::shared_ptr<ManoeuvreList> manoeuvreList = Project::currentPlot->getManoeuvreList();
	
	//Manoeuvre Selection Buttons
	UpDownButtons::Interaction interaction = UpDownButtons::draw("", height / 2.0);
	if(interaction == UpDownButtons::Interaction::UP) manoeuvreList->selectPreviousManoeuvre();
	else if(interaction == UpDownButtons::Interaction::DOWN) manoeuvreList->selectNextManoeuvre();
	
	
	auto selectedManoeuvre = Project::currentPlot->getSelectedManoeuvre();
	
	//Selected Manoeuvre Miniature Display
	ImGui::SameLine();
	glm::vec2 manoeuvreDisplaySize(ImGui::GetTextLineHeight() * 8.0, height);
	bool b_noSelection = selectedManoeuvre == nullptr;
	if(b_noSelection) {
		ImGui::PushFont(Fonts::sansRegular15);
		backgroundText("No Selection", manoeuvreDisplaySize, glm::vec4(0.2, 0.2, 0.2, 1.0));
		ImGui::PopFont();
	}
	else selectedManoeuvre->miniatureGui(manoeuvreDisplaySize);
	
	//Rapid to Start / Rapid to End
	ImGui::SameLine();
	glm::vec2 cursorpos = ImGui::GetCursorPos();
	ImGui::BeginDisabled(b_noSelection || !selectedManoeuvre->canRapidToStart() || selectedManoeuvre->isAtStart());
	bool atStart = !b_noSelection && selectedManoeuvre->isAtStart();
	if(atStart) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
	if(previousButton("RapidToStart", height, true, ImDrawFlags_RoundCornersLeft)) selectedManoeuvre->rapidToStart();
	if(atStart) ImGui::PopStyleColor();
	ImGui::EndDisabled();
	ImGui::SetCursorPos(cursorpos + glm::vec2(height + 1, 0));
	ImGui::BeginDisabled(b_noSelection || !selectedManoeuvre->canRapidToTarget() || selectedManoeuvre->isAtTarget());
	bool atTarget = !b_noSelection && selectedManoeuvre->isAtTarget();
	if(atTarget) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
	if(nextButton("RapidToTarget", height, true, ImDrawFlags_RoundCornersRight)) selectedManoeuvre->rapidToTarget();
	if(atTarget) ImGui::PopStyleColor();
	ImGui::EndDisabled();
	
	
	//Playback Position Display / Control
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::PushFont(Fonts::sansBold12);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(2));
	bool disablePlaybackTimeSetting = b_noSelection || !selectedManoeuvre->canSetPlaybackPosition();
	if(disablePlaybackTimeSetting) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::darkGray);
	}
	static double playbackPosition = 0.0;
	static double remaningPlaybackTime = 0.0;
	if(b_noSelection){
		playbackPosition = 0.0;
		remaningPlaybackTime = 0.0;
	}else{
		playbackPosition = selectedManoeuvre->getPlaybackPosition();
		remaningPlaybackTime = selectedManoeuvre->getRemainingPlaybackTime();
	}
	if(timeEntryWidgetSeconds("##timeFromStart", (height / 2.0) - 1, playbackPosition)) selectedManoeuvre->setPlaybackPosition(playbackPosition);
	if(timeEntryWidgetSeconds("##timeToEnd", (height / 2.0) - 1, remaningPlaybackTime)) selectedManoeuvre->setPlaybackPosition(selectedManoeuvre->getDuration() - remaningPlaybackTime);
	if(disablePlaybackTimeSetting) {
		ImGui::PopItemFlag();
		ImGui::PopStyleColor();
	}
	ImGui::PopStyleVar();
	ImGui::PopFont();
	ImGui::EndGroup();
	
	//Rapid to playback position
	ImGui::SameLine();
	bool atPlayback = !b_noSelection && selectedManoeuvre->isAtPlaybackPosition();
	ImGui::BeginDisabled(b_noSelection || atPlayback || !selectedManoeuvre->canRapidToPlaybackPosition());
	if(atPlayback) ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
	if(buttonArrowDownStop("rapidToPlaybackPosition", height)) selectedManoeuvre->rapidToPlaybackPosition();
	if(atPlayback) ImGui::PopStyleColor();
	ImGui::EndDisabled();
	
	//Start Playback
	ImGui::SameLine();
	if(b_noSelection || !selectedManoeuvre->isPlaying()){
		ImGui::BeginDisabled(b_noSelection || !selectedManoeuvre->canStartPlayback());
		if(buttonPlay("PlayManoeuvre", height)) selectedManoeuvre->startPlayback();
		ImGui::EndDisabled();
	}else{
		ImGui::BeginDisabled(!selectedManoeuvre->canPausePlayback());
		if(buttonPause("PauseManoeuvre", height)) selectedManoeuvre->pausePlayback();
		ImGui::EndDisabled();
	}
	
	//Stop
	ImGui::SameLine();
	ImGui::BeginDisabled(b_noSelection || !selectedManoeuvre->canStop());
	if(buttonStop("StopManoeuvre", height)) selectedManoeuvre->stop();
	ImGui::EndDisabled();
	
	//Stop All
	ImGui::SameLine();
	ImGui::BeginDisabled(!PlaybackManager::isAnyManoeuvreActive());
	if(buttonSTOP("StopAll", height)) PlaybackManager::stopAllManoeuvres();
	ImGui::EndDisabled();

}

void sequencePlaybackControls(float height){}

}
