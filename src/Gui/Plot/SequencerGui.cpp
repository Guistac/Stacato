#include <pch.h>
#include "SequencerGui.h"

#include <iostream>

#include <imgui_internal.h>

#include "SequencerLibrary.h"

#include "Animation/Playback/Playback.h"

#include "Gui/Assets/Fonts.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Plot/Sequencer.h"




/*
enum class MediaType{
	AUDIO = 1,
	VIDEO = 2,
	MOTION = 3,
	MEDIA_COUNT = 4
};

struct Track{
	int id;
	char name[128];
	MediaType type;
};

std::vector<Track> tracks = {
	{1,		"Audio1", 	MediaType::AUDIO },
	{2,		"Audio2", 	MediaType::AUDIO },
	{3,		"Audio3", 	MediaType::AUDIO },
	{4,		"Audio4", 	MediaType::AUDIO },
	{5,		"Video1", 	MediaType::VIDEO },
	{6,		"Video2", 	MediaType::VIDEO },
	{7,		"Video3", 	MediaType::VIDEO },
	{8,		"Motion1", 	MediaType::MOTION },
	{9,		"Motion2", 	MediaType::MOTION },
	{10,	"Motion3", 	MediaType::MOTION }
};

int multiTrackIds[4] = {
	11,
	12,
	13,
	14
};
const char* multiTrackNames[4] = {
	"Translation",
	"Rotation",
	"Oscillation 1",
	"Oscillation 2"
};
MediaType multiTrackTypes[4] = {
	MediaType::MOTION,
	MediaType::MOTION,
	MediaType::VIDEO,
	MediaType::VIDEO
};

struct Region{
	int id;
	int trackId;
	char name[128];
	long long int startTime;
	long long int length;
	MediaType type;
	int linkMasterID;
};

std::vector<Region> regions = {
	{1, 	1, 	"AudioRegion1", 	1000000, 	10000000, 	MediaType::AUDIO, 	1},
	{2, 	2, 	"AudioRegion2", 	2000000, 	2000000, 	MediaType::AUDIO, 	1},
	{3, 	3, 	"AudioRegion3", 	5000000, 	3200000, 	MediaType::AUDIO, 	3},
	{4, 	4, 	"AudioRegion4", 	4000000, 	53400000, 	MediaType::AUDIO, 	-1},
	{5, 	5, 	"VideoRegion1", 	3000000, 	6900000, 	MediaType::VIDEO, 	1},
	{6, 	6, 	"VideoRegion2", 	10000000, 	1200000, 	MediaType::VIDEO, 	6},
	{7, 	7, 	"VideoRegion3", 	8800000, 	3400000, 	MediaType::VIDEO, 	6},
	{8, 	8, 	"MotionRegion1", 	7700000, 	6000000, 	MediaType::MOTION, 	3},
	{9, 	9, 	"MotionRegion2", 	8800000, 	3400000, 	MediaType::MOTION, 	3},
	{10, 	10, "MotionRegion3", 	7700000, 	6000000, 	MediaType::MOTION, 	-1}
};

Sequencer::RegionTrackCompatFun mediaCompatibilityFunction = [](int t1, int t2) -> bool {
	MediaType media1 = (MediaType)t1;
	MediaType media2 = (MediaType)t2;
	if(media1 >= MediaType::MEDIA_COUNT || media2 >= MediaType::MEDIA_COUNT) return false;
	if((media1 == MediaType::AUDIO && media2 == MediaType::MOTION) ||
	   (media2 == MediaType::AUDIO && media1 == MediaType::MOTION)) return true;
	return media1 == media2;
};


struct Event{
	int id;
	int trackId;
	char name[128];
	long long int time;
	MediaType type;
	int linkMasterID;
};

std::vector<Event> events = {
	{100, 1, "Event1", 0, 		MediaType::AUDIO, 	101},
	{101, 4, "Event2", 1000000, MediaType::VIDEO, 	101},
	{102, 8, "Event3", 2000000, MediaType::MOTION, 	3},
	{103, 9, "Event4", 3000000, MediaType::MOTION, 	-1}
};


const char* options [10] = {
	"one",
	"two",
	"three",
	"four",
	"five",
	"six",
	"seven",
	"eight",
	"nine",
	"ten"
};

int selectedOption = 3;






void editor(){
	
	if(!Sequencer::hasContext()) {
		Sequencer::Context* context = Sequencer::createContext();
		context->regionCompatibilityFunction = mediaCompatibilityFunction;
	}
	
	if(Sequencer::begin("Sequencer")){
		
		int counter = 0;
		for(auto& track : tracks){
			
			if(counter == 0) Sequencer::beginTrackGroup("A/V");
			if(counter == 0) Sequencer::beginTrackGroup("Audio");
			if(counter == 4) Sequencer::beginTrackGroup("Video");
			
			switch(track.type){
				case MediaType::AUDIO: Sequencer::setTrackColor({0.3f, 0.4f, 0.1f, 1.0f}); break;
				case MediaType::VIDEO: Sequencer::setTrackColor({0.1f, 0.3f, 0.4f, 1.0f}); break;
				case MediaType::MOTION: Sequencer::setTrackColor({0.4f, 0.3f, 0.1f, 1.0f}); break;
				default: break;
			}
			
			if(Sequencer::beginTrack(track.id, track.name, (int)track.type)){
				
				ImVec2 userspaceSize = Sequencer::getTrackHeaderUserSpaceSize();
				ImVec2 userspacePos = Sequencer::getTrackHeaderUserSpacePos();
				ImGui::SetCursorPos(userspacePos + ImVec2(10, (userspaceSize.y - ImGui::GetFrameHeight()) / 2.0));
				
				if(strcmp(track.name, "Audio3") == 0){
					ImGui::SetItemAllowOverlap();
					ImGui::SetNextItemWidth(userspaceSize.x - 20.0);
					if(ImGui::BeginCombo("##type", options[selectedOption])){
						for(int i = 0; i < 10; i++){
							if(ImGui::Selectable(options[i], i == selectedOption)){
								selectedOption = i;
							}
						}
						ImGui::EndCombo();
					}
				}else{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.6f, 1.0f));
					ImGui::SetItemAllowOverlap();
					if(ImGui::Button("Mute")) std::cout << "mute " << track.name << std::endl;
					ImGui::PopStyleColor();
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.6f, 0.6f, 0.2f, 1.0f));
					ImGui::SetItemAllowOverlap();
					if(ImGui::Button("Solo")) std::cout << "solo " << track.name << std::endl;
					ImGui::PopStyleColor();
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.6f, 0.2f, 0.2f, 1.0f));
					ImGui::SetItemAllowOverlap();
					if(ImGui::Button("Lock")) std::cout << "lock " << track.name << std::endl;
					ImGui::PopStyleColor();
				}
				
				Sequencer::endTrack();
			}
			
			if(counter == 6) Sequencer::endTrackGroup();
			if(counter == 6) Sequencer::endTrackGroup();
			if(counter == 9) Sequencer::endTrackGroup();
			
			counter++;
		}
		
		if(Sequencer::beginMultiTrack("MultiTrack", multiTrackIds, multiTrackNames, 4, (int*)multiTrackTypes)){
			Sequencer::endMultiTrack();
		}
		
		for(auto& region : regions){
			
			switch(region.type){
				case MediaType::AUDIO: Sequencer::setRegionColor({0.2f, 0.7f, 0.3f, 1.0f}); break;
				case MediaType::VIDEO: Sequencer::setRegionColor({0.7f, 0.2f, 0.3f, 1.0f}); break;
				case MediaType::MOTION: Sequencer::setRegionColor({0.1f, 0.3f, 0.6f, 1.0f}); break;
				default: break;
			}
			
			if(Sequencer::beginRegion(region.id, region.trackId, region.name, region.startTime, region.length, (int)region.type, region.linkMasterID)){
				Sequencer::endRegion();
			}
		}
		
		Sequencer::setRegionColor({.6f, .6f, .0f, 1.f});
		for(auto& event : events){
			switch(event.type){
				case MediaType::AUDIO: Sequencer::setRegionColor({0.2f, 0.7f, 0.3f, 1.0f}); break;
				case MediaType::VIDEO: Sequencer::setRegionColor({0.7f, 0.2f, 0.3f, 1.0f}); break;
				case MediaType::MOTION: Sequencer::setRegionColor({0.1f, 0.3f, 0.6f, 1.0f}); break;
				default: break;
			}
			Sequencer::event(event.id, event.trackId, event.name, event.time, (int)event.type, event.linkMasterID);
		}
		Sequencer::end();
	}
	
}


*/

#include "Environnement/Environnement.h"
#include "Machine/Machine.h"

namespace Sequencer::Gui{

std::shared_ptr<SequencerWindow> sequencerWindow = std::make_shared<SequencerWindow>();
std::shared_ptr<SequencerWindow> SequencerWindow::get(){ return sequencerWindow; }

void editor(){
	
	auto sequence = Sequencer::getSequence();
	SequencerLibrary::setContext(sequence->guiContext);
	
	if(SequencerLibrary::begin("Sequencer")){
		
		glm::vec2 sequencerMin(0,0);
		glm::vec2 crossSize(sequence->guiContext->style.trackHeaderWidth, sequence->guiContext->style.timelineHeight);
		float padding = ImGui::GetTextLineHeight() * 0.1;
		glm::vec2 buttonPos(padding);
		float crossWidgetHeight = crossSize.y - 2.0 * padding;
		glm::vec2 addButtonSize(ImGui::CalcTextSize("Add Track").x + ImGui::GetStyle().FramePadding.x * 2.0, crossWidgetHeight);
		ImGui::SetCursorPos(sequencerMin + padding);
		if(ImGui::Button("Add Track", addButtonSize)) ImGui::OpenPopup("SequencerTrackAdder");
		if (ImGui::BeginPopup("SequencerTrackAdder")) {
			ImGui::BeginDisabled();
			ImGui::MenuItem("Add Track");
			ImGui::MenuItem("Machine List :");
			ImGui::EndDisabled();
			ImGui::Separator();
			for (auto& machine : Environnement::getMachines()) {
				if(machine->animatables.empty()) continue;
				if (ImGui::BeginMenu(machine->getName())) {
					for (auto& animatable : machine->animatables) {
						if (animatable->hasParentComposite()) continue;
						bool isSelected = sequence->hasTrack(animatable);
						if (ImGui::MenuItem(animatable->getName(), nullptr, isSelected)) {
							if (!isSelected) sequence->addTrack(animatable);
						}
					}
					ImGui::EndMenu();
				}
			}
			ImGui::EndPopup();
		}
		
		
		auto& tracks = sequence->getTracks();
		for(int i = 0; i < tracks.size(); i++){
			if(SequencerLibrary::beginTrack(i, tracks[i]->animatable->getName())){
				
				SequencerLibrary::endTrack();
			}
		}
		
		
		SequencerLibrary::end();
	}
}







	void transportControls(float height){
		auto context = SequencerLibrary::getContext();
		
		ImGui::PushFont(Fonts::sansBold26);
		timeEntryWidgetMicroseconds("##SequencerTime", height, context->playbackTime);
		ImGui::PopFont();
		
		ImGui::SameLine();
		
		if(previousButton("PreviousManoeuvre", height, true, ImDrawFlags_RoundCornersLeft)) {}
		ImGui::SameLine();
		if(nextButton("NextManoeuvre", height, true, ImDrawFlags_RoundCornersRight)) {}
		
		ImGui::SameLine();
		if(buttonPause("##Pause", height)){}
		ImGui::SameLine();
		if(buttonPlay("##Play", height)){}
	}





};

