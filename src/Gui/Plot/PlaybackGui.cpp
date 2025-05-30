#include <pch.h>
#include "PlaybackGui.h"

#include <imgui.h>

#include "Plot/Plot.h"
#include "Plot/ManoeuvreList.h"
#include "Animation/Manoeuvre.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Animation/Playback/Playback.h"

#include "Animation/Animation.h"
#include "Machine/Machine.h"

#include "Stacato/StacatoEditor.h"
#include "Stacato/Project/StacatoProject.h"

namespace Playback::Gui{

void manoeuvrePlaybackControls(float height){
	
	if(!Stacato::Editor::hasCurrentProject()){
		ImGui::Text("No Project Loaded");
		return;
	}
	
	auto currentPlot = Stacato::Editor::getCurrentProject()->getCurrentPlot();
	std::shared_ptr<ManoeuvreList> manoeuvreList = currentPlot->getManoeuvreList();
	
	//Manoeuvre Selection Buttons
	UpDownButtons::Interaction interaction = UpDownButtons::draw("", height / 2.0);
	if(interaction == UpDownButtons::Interaction::UP) manoeuvreList->selectPreviousManoeuvre();
	else if(interaction == UpDownButtons::Interaction::DOWN) manoeuvreList->selectNextManoeuvre();
	
	
	auto selectedManoeuvre = currentPlot->getSelectedManoeuvre();
	
	//Selected Manoeuvre Miniature Display
	ImGui::SameLine();
	glm::vec2 manoeuvreDisplaySize(ImGui::GetTextLineHeight() * 16.0, height);
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
	static bool b_hasPlaybackPosition = false;
	static double playbackPosition = 0.0;
	static double remaningPlaybackTime = 0.0;
	if(b_noSelection){
		b_hasPlaybackPosition = false;
		playbackPosition = 0.0;
		remaningPlaybackTime = 0.0;
	}else{
		playbackPosition = selectedManoeuvre->getSychronizedPlaybackPosition();
		b_hasPlaybackPosition = !isnan(playbackPosition);
		remaningPlaybackTime = selectedManoeuvre->getRemainingPlaybackTime();
	}
	
	if(b_hasPlaybackPosition){
		if(timeEntryWidgetSeconds("##timeFromStart", (height / 2.0) - 1, playbackPosition)) selectedManoeuvre->setSynchronizedPlaybackPosition(playbackPosition);
		if(timeEntryWidgetSeconds("##timeToEnd", (height / 2.0) - 1, remaningPlaybackTime)) selectedManoeuvre->setSynchronizedPlaybackPosition(selectedManoeuvre->getDuration() - remaningPlaybackTime);
	}else{
		static double pos = 0.0;
		if(timeEntryWidgetSeconds("##timeFromStart", (height / 2.0) - 1, pos, "+xx:xx:xx.x")) selectedManoeuvre->setSynchronizedPlaybackPosition(pos);
		if(timeEntryWidgetSeconds("##timeToEnd", (height / 2.0) - 1, pos, "-xx:xx:xx.x")) selectedManoeuvre->setSynchronizedPlaybackPosition(selectedManoeuvre->getDuration() - pos);
	}
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
	ImGui::BeginDisabled(b_noSelection || !selectedManoeuvre->canStartPlayback());
	if(buttonPlay("PlayManoeuvre", height)) selectedManoeuvre->startPlayback();
	ImGui::EndDisabled();
	
	//Pause Playback
	ImGui::SameLine();
	ImGui::BeginDisabled(b_noSelection || !selectedManoeuvre->canPausePlayback());
	if(buttonPause("PauseManoeuvre", height)) selectedManoeuvre->pausePlayback();
	ImGui::EndDisabled();
	
	//Stop
	ImGui::SameLine();
	ImGui::BeginDisabled(b_noSelection || !selectedManoeuvre->canStop());
	if(buttonStop("StopManoeuvre", height)) selectedManoeuvre->stop();
	ImGui::EndDisabled();
	
	//Stop All
	ImGui::SameLine();
	ImGui::BeginDisabled(!PlaybackManager::isAnyAnimationActive());
	if(buttonSTOP("StopAll", height)) PlaybackManager::stopAllAnimations();
	ImGui::EndDisabled();

}

void sequencerPlaybackControls(float height){}








void PlaybackManagerWindow::onDraw(){

	ImGui::PushFont(Fonts::sansBold15);
	backgroundText("Playback Speed Adjustement:", ImVec2(ImGui::GetContentRegionAvail().x, 0), Colors::darkGray, Colors::white);
	ImGui::PopFont();
	if(ImGui::Button("Reset")) PlaybackManager::resetPlaybackSpeedMultiplier();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	float playbackSpeedMutliplier = (PlaybackManager::getPlaybackSpeedMultiplier() - 1.0) * 100.0;
	float minMultiplier = (PlaybackManager::getMinPlaybackSpeedMultiplier() - 1.0) * 100.0;
	float maxMultiplier = (PlaybackManager::getMaxPlaybackSpeedMutliplier() - 1.0) * 100.0;
	if(ImGui::SliderFloat("##PlaybackSpeed", &playbackSpeedMutliplier, minMultiplier, maxMultiplier, "%.1f%%")){
		PlaybackManager::setPlaybackSpeedMultiplier((playbackSpeedMutliplier / 100.0) + 1.0);
	}
	
	ImGui::Separator();
	
	auto drawing = ImGui::GetWindowDrawList();
	ImDrawListSplitter drawingLayers;
	
	glm::vec2 padding = ImGui::GetStyle().FramePadding;
	float boxWidth = ImGui::GetContentRegionAvail().x;
	float contentWidth = boxWidth - padding.x * 2.0;
	float boxStartX = ImGui::GetCursorPosX();
	
	

	for(auto& manoeuvre : PlaybackManager::getActiveManoeuvres()){

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
		
		drawingLayers.Split(drawing, 2);
		drawingLayers.SetCurrentChannel(drawing, 1);
		
		glm::vec2 cursor = ImGui::GetCursorPos();
		ImGui::BeginGroup();
		ImGui::SetCursorPos(cursor + padding);
		ImGui::BeginGroup();
		
		glm::vec2 contentCursorPosition = ImGui::GetCursorPos();
		
		if(buttonStop("stopManoeuvre")) manoeuvre->stop();
		
		ImGui::SameLine();
		ImGui::BeginDisabled(!manoeuvre->canPausePlayback());
		if(buttonPause("PauseManoeuvre")) manoeuvre->pausePlayback();
		ImGui::EndDisabled();
		
		ImGui::SameLine();
		ImGui::BeginDisabled(!manoeuvre->canStartPlayback());
		if(buttonPlay("ResumeManoeuvre")) manoeuvre->startPlayback();
		ImGui::EndDisabled();
		
		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0);
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%s", manoeuvre->getName());
		ImGui::PopFont();
		
		ImGui::SetCursorPos(contentCursorPosition);
		ImGui::Dummy(glm::vec2(ImGui::GetFrameHeight()));
		
		ImGui::Dummy(ImVec2(contentWidth, 0));
		drawing->AddLine(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(Colors::gray));
		
		auto activeAnimations = manoeuvre->getActiveAnimations();
		for(int i = 0; i < activeAnimations.size(); i++){
			ImGui::PushID(i);
			
			auto& animation = activeAnimations[i];
			auto animatable = animation->getAnimatable();
			auto machine = animatable->getMachine();
			
			if(buttonStop("stop")) animation->stop();
			
			if(animation->canPausePlayback()){
				ImGui::SameLine();
				if(buttonPause("pause")) animation->pausePlayback();
			}
			if(animation->isPaused()){
				ImGui::SameLine();
				ImGui::BeginDisabled(!animation->canStartPlayback());
				if(buttonPlay("resume")) animation->startPlayback();
				ImGui::EndDisabled();
			}
			
			ImGui::SameLine();
			glm::vec2 progressBarCursorPosition = ImGui::GetCursorPos();
			glm::vec2 progressBarSize(ImGui::GetContentRegionAvail().x - padding.x, ImGui::GetFrameHeight());
			ImGui::Dummy(progressBarSize);
			glm::vec2 min = ImGui::GetItemRectMin();
			glm::vec2 max = ImGui::GetItemRectMax();
			drawing->AddRectFilled(min, max, ImColor(Colors::black), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
			
			float progress;
			if(animation->isInRapid()) progress = animation->getRapidProgress();
			else progress = animation->getPlaybackProgress();
			if(progress > 0.0 && progress <= 1.0){
				glm::vec2 maxPlayback(min.x + (max.x - min.x) * progress, max.y);
				drawing->AddRectFilled(min, maxPlayback, ImColor(1.f, 1.f, 1.f, .2f), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
			}
			
			ImGui::SetCursorPosX(progressBarCursorPosition.x + ImGui::GetStyle().FramePadding.x);
			ImGui::SetCursorPosY(progressBarCursorPosition.y + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0);
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("%s", machine->getName());
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("%s", animatable->getName());
			
			ImGui::SetCursorPos(progressBarCursorPosition);
			ImGui::Dummy(progressBarSize);
			
			ImGui::PopID();
			
		}
		
		
		if(!manoeuvre->isInRapid()){
			double realRemainingTime = manoeuvre->getRemainingPlaybackTime() / PlaybackManager::getPlaybackSpeedMultiplier();
			std::string realReamingTimeString = TimeStringConversion::secondsToTimecodeString(realRemainingTime);
			ImGui::Text("Remaining Time: %s", realReamingTimeString.c_str());
		}
		
		ImGui::EndGroup();
		glm::vec2 contentSize = ImGui::GetItemRectSize();
		glm::vec2 widgetSize(ImGui::GetContentRegionAvail().x, contentSize.y + padding.y * 2.0);
		ImGui::SetCursorPos(cursor + widgetSize);
		ImGui::EndGroup();
	
		drawingLayers.SetCurrentChannel(drawing, 0);
		auto min = ImGui::GetItemRectMin();
		auto max = ImGui::GetItemRectMax();
		
		drawing->AddRectFilled(min, max, ImColor(Colors::almostBlack), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		
		drawingLayers.Merge(drawing);
		
		ImGui::PopStyleVar();
	}
	
	auto activeAnimations = PlaybackManager::getActiveAnimations();
	for(auto& animation : activeAnimations){
		if(animation->hasManoeuvre()) continue;
		
		ImGui::BeginGroup();
		
		ImGui::PushFont(Fonts::sansBold15);
		auto animatable = animation->getAnimatable();
		auto machine = animatable->getMachine();
		ImGui::Text("%s : %s", machine->getName(), animatable->getName());
		float progress = animation->getPlaybackPosition() / animation->getDuration();
		ImGui::PopFont();
		
		ImGui::EndGroup();
	}
	
	if(PlaybackManager::getActiveAnimations().empty() && PlaybackManager::getActiveManoeuvres().empty()){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("No Manoeuvres are playing.");
		ImGui::PopFont();
		ImGui::TextWrapped("Active manoeuvres & animations will show up once they are started.");
		ImGui::PopStyleColor();
	}
	
}


}
