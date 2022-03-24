#include <pch.h>

#include "Motion/Manoeuvre/Manoeuvre.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Environnement/Environnement.h"
#include "Motion/Machine/Machine.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Motion/Curve/Curve.h"
#include "Plot/Plot.h"

#include "Motion/Playback.h"


void Manoeuvre::listGui(const std::shared_ptr<Manoeuvre>& manoeuvre) {

	ImGui::BeginGroup();

	ImGui::PushFont(Fonts::robotoBold20);
	float cueNameWidth = ImGui::CalcTextSize(manoeuvre->name).x;
	ImGui::PopFont();
	ImGui::PushFont(Fonts::robotoLight20);
	const char* manoeuvreTypeShortName = manoeuvre->getShortTypeString();
	float typeNameWidth = ImGui::CalcTextSize(manoeuvreTypeShortName).x;
	ImGui::PopFont();

	float headerStripWidth = std::max(cueNameWidth, typeNameWidth) + ImGui::GetStyle().ItemSpacing.x;
	glm::vec2 min = ImGui::GetWindowPos();
	glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
	glm::vec4 headerStripColor;
	switch (manoeuvre->type) {
		case Manoeuvre::Type::KEY_POSITION:
			headerStripColor = Colors::darkYellow;
			break;
		case Manoeuvre::Type::TIMED_MOVEMENT:
			headerStripColor = Colors::darkGray;
			break;
		case Manoeuvre::Type::MOVEMENT_SEQUENCE:
			headerStripColor = Colors::darkRed;
			break;
	}
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(headerStripColor), 10.0, ImDrawFlags_RoundCornersLeft);

	if (!manoeuvre->b_valid) {
		bool blink = (int)Timing::getProgramTime_milliseconds() % 1000 > 500;
		ImGui::PushStyleColor(ImGuiCol_Text, blink ? Colors::red : Colors::yellow);
	}
	else ImGui::PushStyleColor(ImGuiCol_Text, Colors::white);
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
	ImGui::Text("%s", manoeuvre->name);
	ImGui::PopFont();
	ImGui::PopStyleColor();

	ImGui::PushFont(Fonts::robotoLight20);
	ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 1.0, 1.0, 0.3));
	ImGui::NewLine();
	ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
	ImGui::Text("%s", manoeuvreTypeShortName);
	ImGui::PopStyleColor();
	ImGui::PopFont();

	ImGui::EndGroup();

	ImGui::SameLine();
	ImGui::Text("%s", manoeuvre->description);

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
	if (Playback::isPrimedToStart(manoeuvre)) {
		ImGui::Text("Primed");
	}
	if (Playback::isPlaying(manoeuvre)) {
		ImGui::SameLine();
		ImGui::Text("Playing");
		glm::vec2 min = ImGui::GetWindowPos();
		glm::vec2 windowSize = ImGui::GetWindowSize();
		float progress = manoeuvre->getPlaybackProgress();
		glm::vec2 max(min.x + windowSize.x * progress, min.y + windowSize.y);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.4)), 5.0);
	}
}


void Manoeuvre::editGui(const std::shared_ptr<Manoeuvre>& manoeuvre) {

	ImGui::PushFont(Fonts::robotoBold42);
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 3.0);
	ImGui::InputText("##cueName", manoeuvre->name, 64);
	ImGui::PopFont();
	ImGui::SameLine();
	glm::vec2 descriptionFieldSize(ImGui::GetContentRegionAvail().x, ImGui::GetItemRectSize().y);
	ImGui::PushFont(Fonts::robotoRegular20);
	ImGui::InputTextMultiline("##cueDescription", manoeuvre->description, 256, descriptionFieldSize, ImGuiInputTextFlags_CtrlEnterForNewLine);
	ImGui::PopFont();

	bool refreshAllTracks = false;

	if (ImGui::BeginCombo("##manoeuvreTypeSelector", Enumerator::getDisplayString(manoeuvre->type))) {
		for (auto& type : Enumerator::getTypes<Manoeuvre::Type>()) {
			if(type.enumerator == Manoeuvre::Type::TIMED_MOVEMENT) continue;
			if (ImGui::Selectable(type.displayString, manoeuvre->type == type.enumerator)) {
				manoeuvre->setType(type.enumerator);
				refreshAllTracks = true;
			}
		}
		ImGui::EndCombo();
	}

	if (refreshAllTracks) {
		for (auto& track : manoeuvre->tracks) {
			track->refreshAfterParameterEdit();
		}
	}

    if(ImGui::BeginTabBar("##ManoeuvreEditorTabBar")){
        if(ImGui::BeginTabItem("Track Sheet")){
            ImGui::BeginChild("TrackSheet");
            trackSheetGui(manoeuvre);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Curve Editor")){
            ImGui::BeginChild("CurveEditor");
            curveEditorGui(manoeuvre);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
		/*
        if(ImGui::BeginTabItem("Space Editor")){
            ImGui::BeginChild("SpaceEditor");
            spatialEditorGui(manoeuvre);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
		*/
        ImGui::EndTabBar();
    }
}


void Manoeuvre::trackSheetGui(const std::shared_ptr<Manoeuvre>& manoeuvre){
    
    bool manoeuvreIsPlaying = Playback::isPlaying(manoeuvre);

    std::shared_ptr<ParameterTrack> removedTrack = nullptr;
    std::shared_ptr<ParameterTrack> movedUpTrack = nullptr;
    std::shared_ptr<ParameterTrack> movedDownTrack = nullptr;

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_ScrollX | ImGuiTableFlags_NoHostExtendY;

    int columnCount;
    switch (manoeuvre->type) {
        case Manoeuvre::Type::KEY_POSITION:
            columnCount = 6;
            break;
        default:
            columnCount = 8;
            break;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
	
    if (ImGui::BeginTable("##parameters", columnCount, tableFlags)) {

        ImGui::TableSetupColumn("Manage");
        ImGui::TableSetupColumn("Machine");
        ImGui::TableSetupColumn("Parameter");
        if (manoeuvre->type != Manoeuvre::Type::KEY_POSITION) {
            if (manoeuvre->type == Manoeuvre::Type::TIMED_MOVEMENT)
                ImGui::TableSetupColumn("Interpolation");
            else ImGui::TableSetupColumn("Movement");
            ImGui::TableSetupColumn("Origin");
            ImGui::TableSetupColumn("Target");
            ImGui::TableSetupColumn("Timing");
            ImGui::TableSetupColumn("Ramps");
        }
        else {
            ImGui::TableSetupColumn("Previous");
            ImGui::TableSetupColumn("Target");
            ImGui::TableSetupColumn("Next");
        }

        ImGui::TableHeadersRow();







        //============ BEGIN ROW LAMBDA =============

        static auto parameterTrackRowGui = [&](std::shared_ptr<ParameterTrack>& parameterTrack) {

            bool trackEdited = false;
            bool chainingDependenciesEdited = false;

            ImGui::PushID(parameterTrack->parameter->name);
            ImGui::PushID(parameterTrack->parameter->machine->getName());

            ImGui::TableNextRow(ImGuiTableRowFlags_None);
                
			ImGui::BeginDisabled(manoeuvreIsPlaying);

            //====== Track Management Column ======
            ImGui::TableSetColumnIndex(0);

            if (!parameterTrack->hasParentParameterTrack()) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.1));
                if (buttonCross("##remove")) removedTrack = parameterTrack;
                ImGui::SameLine();
                bool disableMoveUp = parameterTrack == manoeuvre->tracks.front();
				ImGui::BeginDisabled(disableMoveUp);
				if (ImGui::ArrowButton("##moveUp", ImGuiDir_Up)) movedUpTrack = parameterTrack;
				ImGui::EndDisabled();
				ImGui::SameLine();
                bool disableMoveDown = parameterTrack == manoeuvre->tracks.back();
				ImGui::BeginDisabled(disableMoveDown);
				if (ImGui::ArrowButton("##moveDown", ImGuiDir_Down)) movedDownTrack = parameterTrack;
				ImGui::EndDisabled();
				ImGui::PopStyleVar();
            }

            ImGui::PushStyleColor(ImGuiCol_Text, Colors::white);
            ImGui::PushFont(Fonts::robotoBold15);
            bool parameterValid = parameterTrack->b_valid;
            if (!parameterValid) ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);

            //====== Machine Column ======
            ImGui::TableNextColumn();
            if (!parameterTrack->hasParentParameterTrack()) {
                ImGui::Text("%s", parameterTrack->parameter->machine->getName());
            }
			
			if(ImGui::IsItemHovered()){
				ImGui::BeginTooltip();
				
				
				if(!parameterTrack->b_valid) ImGui::Text("Parameter Track not Valid");
				
				for(auto& curve : parameterTrack->curves){
					
					if(!curve->b_valid) ImGui::Text("- Curve Not Valid");
					
					for(auto& controlPoint : curve->points){
						if(!controlPoint->b_valid) ImGui::Text("-- Control Point Not Valid : %s", Enumerator::getDisplayString(controlPoint->validationError));
					}
					
					for(auto& interpolation : curve->interpolations){
						if(!interpolation->b_valid) ImGui::Text("-- Interpolation Not Valid : %s", Enumerator::getDisplayString(interpolation->validationError));
					}
					
				}
				
				
				
				ImGui::EndTooltip();
			}

            //====== Parameter Column ======
            ImGui::TableNextColumn();
            ImGui::Text("%s", parameterTrack->parameter->name);

            if (!parameterValid) ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopFont();

            if (!parameterTrack->hasChildParameterTracks()) {

                //====== Movement Column ======

                if (manoeuvre->type != Manoeuvre::Type::KEY_POSITION) {
                    ImGui::TableNextColumn();

                    //--- Interpolation Selector ---
                    ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
                    trackEdited |= parameterTrack->interpolationTypeSelectorGui();

                    //--- Sequence Type Selector ---
                    if (manoeuvre->type == Manoeuvre::Type::MOVEMENT_SEQUENCE) {
                        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
                        trackEdited |= parameterTrack->sequenceTypeSelectorGui();
                    }
                }

                float originTargetInputFieldWidth = ImGui::GetTextLineHeight() * 8.0;

                //====== Origin Column ======

                if (manoeuvre->type != Manoeuvre::Type::KEY_POSITION) {
                    ImGui::TableNextColumn();

                    if (parameterTrack->sequenceType != SequenceType::Type::CONSTANT) {
                        //--- Origin Input ---
                        trackEdited |= parameterTrack->originInputGui(originTargetInputFieldWidth);
                    }
                    else ImGui::Dummy(glm::vec2(originTargetInputFieldWidth, ImGui::GetFrameHeight()));

                    //--- Chain Previous ---
                    bool disablePreviousChaining = manoeuvre->type == Manoeuvre::Type::TIMED_MOVEMENT;
                    if (disablePreviousChaining) ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    chainingDependenciesEdited |= parameterTrack->chainPreviousGui(originTargetInputFieldWidth);
                    if (disablePreviousChaining) ImGui::PopItemFlag();
                }


                if (manoeuvre->type == Manoeuvre::Type::KEY_POSITION) {
                    float keyPositionChainingGuiWidh = ImGui::GetTextLineHeight() * 6.0;

                    //--- Previous Chained
                    ImGui::TableNextColumn();
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    chainingDependenciesEdited |= parameterTrack->chainPreviousGui(keyPositionChainingGuiWidh);
                    ImGui::PopItemFlag();

                    //--- Target Input ----
                    ImGui::TableNextColumn();
                    trackEdited |= parameterTrack->targetInputGui(originTargetInputFieldWidth);

                    //--- Next Chained ---
                    ImGui::TableNextColumn();
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    chainingDependenciesEdited |= parameterTrack->chainNextGui(keyPositionChainingGuiWidh);
                    ImGui::PopItemFlag();

                }
                else {

                    //====== Target Column ======
                    ImGui::TableNextColumn();
                    //--- Target Input ---
                    trackEdited |= parameterTrack->targetInputGui(originTargetInputFieldWidth);
                    //--- Chain Next ---
                    chainingDependenciesEdited |= parameterTrack->chainNextGui(originTargetInputFieldWidth);
                }


                //====== Timing Column ======

                if (manoeuvre->type != Manoeuvre::Type::KEY_POSITION) {
                    ImGui::TableNextColumn();
                    float timingcolumnWidth = ImGui::GetTextLineHeight() * 7.0;

                    //--- Movement Time Input ---
                    ImGui::SetNextItemWidth(timingcolumnWidth);
                    trackEdited |= parameterTrack->timeInputGui();

                    if (manoeuvre->type != Manoeuvre::Type::TIMED_MOVEMENT) {
                        //--- Time Offset Input ---
                        ImGui::SetNextItemWidth(timingcolumnWidth);
                        trackEdited |= parameterTrack->timeOffsetInputGui();
                    }

                    //====== Ramps Column ======
                    ImGui::TableNextColumn();
                    trackEdited |= parameterTrack->rampInputGui(ImGui::GetTextLineHeight() * 7.0);
                }
            }

			ImGui::EndDisabled();

			ImGui::PopID();
            ImGui::PopID();

            if (chainingDependenciesEdited) manoeuvre->parentPlot->refreshChainingDependencies();
            else if (trackEdited) parameterTrack->refreshAfterParameterEdit();
        };

        //============== END ROW LAMBDA =============






        for (auto& parameterTrack : manoeuvre->tracks) {
            //child parameter tracks are listed in the manoeuvres track vector
            //but they are drawn by the parent parameter group, so we skip them here
            //and draw them after the parameter group
            if (parameterTrack->hasParentParameterTrack()) continue;
            parameterTrackRowGui(parameterTrack);
            //draw the groups child parameter tracks
            if (parameterTrack->hasChildParameterTracks()) {
                for (auto& childParameterTrack : parameterTrack->childParameterTracks) {
                    parameterTrackRowGui(childParameterTrack);
                }
            }
        }

        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        glm::vec2 addTrackButtonSize(ImGui::GetFrameHeight() * 3.0 + ImGui::GetStyle().ItemSpacing.x * 2.0, ImGui::GetFrameHeight());
        if (ImGui::Button("Add Track", addTrackButtonSize)) ImGui::OpenPopup("ManoeuvreTrackAdder");
        if (ImGui::BeginPopup("ManoeuvreTrackAdder")) {
			ImGui::BeginDisabled();
			ImGui::MenuItem("Add Parameter Track");
			ImGui::MenuItem("Machine List :");
			ImGui::EndDisabled();
			ImGui::Separator();
            for (auto& machine : Environnement::getMachines()) {
				if(machine->animatableParameters.empty()) continue;
                if (ImGui::BeginMenu(machine->getName())) {
                    for (auto& parameter : machine->animatableParameters) {
                        if (parameter->hasParentGroup()) continue;
                        bool isSelected = manoeuvre->hasTrack(parameter);
                        if (ImGui::MenuItem(parameter->name, nullptr, isSelected)) {
                            if (!isSelected) manoeuvre->addTrack(parameter);
                            else manoeuvre->removeTrack(parameter);
                        }
                    }
                    ImGui::EndMenu();
                }
            }
            ImGui::EndPopup();
        }
        if(manoeuvre->tracks.empty()){
            ImGui::TableNextColumn();
            ImGui::Text("No Tracks");
        }

        ImGui::EndTable();
        
    }

    ImGui::PopStyleVar(2); //cell padding & item spacing in editing table

    if (removedTrack) manoeuvre->removeTrack(removedTrack->parameter);
    else if (movedUpTrack) {
        int oldIndex;
        for (int i = 0; i < manoeuvre->tracks.size(); i++) {
            if (manoeuvre->tracks[i] == movedUpTrack) {
                oldIndex = i;
                break;
            }
        }
        int newIndex = oldIndex - 1;
        if (newIndex >= 0) {
            manoeuvre->tracks.erase(manoeuvre->tracks.begin() + oldIndex);
            manoeuvre->tracks.insert(manoeuvre->tracks.begin() + newIndex, movedUpTrack);
        }
    }
    else if (movedDownTrack) {
        int oldIndex;
        for (int i = 0; i < manoeuvre->tracks.size(); i++) {
            if (manoeuvre->tracks[i] == movedDownTrack) {
                oldIndex = i;
                break;
            }
        }
        int newIndex = oldIndex + 1;
        if (newIndex < manoeuvre->tracks.size()) {
            manoeuvre->tracks.erase(manoeuvre->tracks.begin() + oldIndex);
            manoeuvre->tracks.insert(manoeuvre->tracks.begin() + newIndex, movedDownTrack);
        }
    }
}

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

void Manoeuvre::spatialEditorGui(const std::shared_ptr<Manoeuvre>& manoeuvre){
    ImGui::Text("2D or 3D view of trajectory editor with timeline");
}




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


/*
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
*/


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

			/*
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
			*/
			
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
				*/
			}
			else {
				/*
				if (!Playback::isPrimedToPlaybackPosition(manoeuvre)) {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::orange);
					if (ImGui::Button("Rapid to Playback Position", singleButtonSize)) Playback::rapidToPlaybackPosition(manoeuvre);
					ImGui::PopStyleColor();
				}
				 */
				bool disableStart = !Playback::isPrimedToStart(manoeuvre);
				if(disableStart) {
					BackgroundText::draw("Not At Sequence Start", singleButtonSize, Colors::gray);
				}else {
					ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
					if (ImGui::Button("Start Playback", singleButtonSize)) Playback::startPlayback(manoeuvre);
					ImGui::PopStyleColor();
				}

		

			}

		break;
	}
}

float Manoeuvre::getPlaybackControlGuiHeight(const std::shared_ptr<Manoeuvre>& manoeuvre) {
	switch (manoeuvre->type) {
		case Manoeuvre::Type::KEY_POSITION:
			return ImGui::GetTextLineHeight() * 2.0 //single row of buttons
				+ ImGui::GetStyle().ItemSpacing.y;	//spacing
		case Manoeuvre::Type::TIMED_MOVEMENT:
			return ImGui::GetTextLineHeight() * 2.0 * 2.0	//single row of buttons
				+ ImGui::GetStyle().ItemSpacing.y * 2.0;	//spacing
		case Manoeuvre::Type::MOVEMENT_SEQUENCE:
			/*
			return ImGui::GetTextLineHeight() * 2.0 * 2.0 //two rows of buttons
				+ ImGui::GetFrameHeight()			//one row of widgets
				+ImGui::GetStyle().ItemSpacing.y * 3.0;	//three spacings
			 */
			return ImGui::GetTextLineHeight() * 2.0 * 2.0 //two rows of buttons
				+ImGui::GetStyle().ItemSpacing.y * 2.0;	//two spacings
		default: return 0.0;
	}
}
