#include <pch.h>

#include <imgui.h>
#include <implot.h>
#include <GLFW/glfw3.h>

#include "Animation/Manoeuvre.h"
#include "Animation/Animation.h"
#include "Animation/Animatable.h"
#include "Machine/Machine.h"

#include "Project/Project.h"

#include "Gui/Utilities/CustomWidgets.h"

void Manoeuvre::curveEditor(){
	
	//——————————— Curve List and point editor ——————————————
	
	float sequenceEditorWidth = ImGui::GetTextLineHeight() * 10.0;
	float sequenceEditorHeight = ImGui::GetContentRegionAvail().y;
	ImGui::BeginChild("SequenceEditor", glm::vec2(sequenceEditorWidth, sequenceEditorHeight));
	
	if(!Project::isPlotEditLocked()){
	
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetStyle().ItemSpacing.y));
		float inputFieldWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * .5f;
		
		glm::vec2 fieldsize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		ImColor backgroundColor = ImColor(Colors::veryDarkGray);
		float rounding = ImGui::GetStyle().FrameRounding;

		auto pointFieldEditor = [&](const char* name, double* data, const char* txt = "") -> bool {
			glm::vec2 cursor = ImGui::GetCursorPos();
			ImGui::Dummy(fieldsize);
			drawing->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), backgroundColor, rounding, ImDrawFlags_RoundCornersAll);
			ImGui::SetCursorPosX(cursor.x + ImGui::GetStyle().FramePadding.x);
			ImGui::SetCursorPosY(cursor.y + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) * .5f);
			ImGui::Text("%s", name);
			ImGui::SameLine();
			ImGui::SetCursorPosY(cursor.y);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if(data == nullptr) {
				ImGui::BeginDisabled();
				ImGui::InputText(name, (char*)txt, 0);
				ImGui::EndDisabled();
				return false;
			}
			else {
				ImGui::SetItemAllowOverlap();
				return ImGui::InputDouble(name, data);
			}
		};
		
		ImGui::PushFont(Fonts::sansBold15);
		backgroundText("Control Point Editor", glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()), Colors::darkGray);
		ImGui::PopFont();
		
		auto& selectedControlPoints = getSelectedControlPoints();
		if(selectedControlPoints.empty()){
			static const char* noSelectionString = "No Selection";
			pointFieldEditor("Time", nullptr, noSelectionString);
			pointFieldEditor("Position", nullptr, noSelectionString);
			pointFieldEditor("Velocity", nullptr, noSelectionString);
			pointFieldEditor("In-Acceleration", nullptr, noSelectionString);
			pointFieldEditor("Out-Acceleration", nullptr, noSelectionString);
		}else if(selectedControlPoints.size() > 1){
			static const char* multipleSelectionString = "Multiple";
			pointFieldEditor("Time", nullptr, multipleSelectionString);
			pointFieldEditor("Position", nullptr, multipleSelectionString);
			pointFieldEditor("Velocity", nullptr, multipleSelectionString);
			pointFieldEditor("In-Acceleration", nullptr, multipleSelectionString);
			pointFieldEditor("Out-Acceleration", nullptr, multipleSelectionString);
		}else{
			//single selection
			auto controlPoint = selectedControlPoints.front();
			bool b_pointEdited = false;
			if(pointFieldEditor("Time", &controlPoint->time)) b_pointEdited = true;
			if(pointFieldEditor("Position", &controlPoint->position)) b_pointEdited = true;
			if(pointFieldEditor("Velocity", &controlPoint->velocity)) b_pointEdited = true;
			if(pointFieldEditor("In-Acceleration", &controlPoint->inAcceleration)) b_pointEdited = true;
			if(pointFieldEditor("Out-Acceleration", &controlPoint->outAcceleration)) b_pointEdited = true;
			if(b_pointEdited) {
				//check for nullptr
				if(getSelectedEditorAnimation()->getType() == ManoeuvreType::SEQUENCE){
					getSelectedEditorAnimation()->toSequence()->updateAfterCurveEdit();
				}
			}
		}
		
		ImGui::PopStyleVar();
		
		ImGui::Separator();
			
	}
	
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	ImGui::PushFont(Fonts::sansBold15);
	backgroundText("Curve Selector", glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()), Colors::darkGray);
	ImGui::PopFont();
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * .1f));
	bool b_curveSelectorHovered = false;
	
	for(int a = 0; a < getAnimations().size(); a++){
		ImGui::PushID(a);
		auto& animation = getAnimations()[a];
		int curveCount = animation->getAnimatable()->getCurveCount();
		
		for(int c = 0; c < curveCount; c++){
			ImGui::PushID(c);
			auto& curve = animation->getCurves()[c];
			
			glm::vec2 lineStartCursor = ImGui::GetCursorPos();
			ImGui::InvisibleButton("background", glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()));
			glm::vec2 min = ImGui::GetItemRectMin();
			glm::vec2 max = ImGui::GetItemRectMax();
			
			ImGui::SetCursorPosY(lineStartCursor.y);
			ImGui::SetCursorPosX(lineStartCursor.x + ImGui::GetFrameHeight());
			ImGui::SetItemAllowOverlap();
			if(ImGui::InvisibleButton("SelectionCatcher", glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()))){
				selectEditorCurve(animation, curve);
			}
			if(ImGui::IsItemHovered()) b_curveSelectorHovered = true;
			
			ImColor itemColor;
			if(isCurveSelectedInEditor(curve)) itemColor = ImColor(Colors::green);
			else if(ImGui::IsItemActive()) itemColor = ImColor(Colors::almostBlack);
			else if(ImGui::IsItemHovered()) itemColor = ImColor(Colors::darkGray);
			else itemColor = ImColor(Colors::veryDarkGray);
			drawing->AddRectFilled(min, max, itemColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
			
			ImGui::SetCursorPos(lineStartCursor);
			ImGui::SetItemAllowOverlap();
			ImGui::Checkbox("##visible", &curve->b_visibleInEditor);
			
			ImGui::SameLine(0.f, ImGui::GetStyle().FramePadding.x);
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("%s", animation->getAnimatable()->getMachine()->getName());
			ImGui::PopFont();
			
			ImGui::SameLine(0.f, ImGui::GetStyle().ItemSpacing.y);
			ImGui::Text("%s", animation->getAnimatable()->getName());
			
			ImGui::PopID();
		}
		ImGui::PopID();
	}
	
	ImGui::PopStyleVar();
	
	if(ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && !b_curveSelectorHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) clearEditorCurveSelection();
	
	ImGui::EndChild();
	
	
	
	ImGui::SameLine();
	verticalSeparator(ImGui::GetStyle().ItemSpacing.x, true, false);
		
	
	//—————————— Curve Editor ———————————
	
	if (shouldRefocusCurves()) {
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
	
	ImPlotFlags plotFlags = ImPlotFlags_AntiAliased | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoChild | ImPlotFlags_NoLegend;
	
	if (ImPlot::BeginPlot("##SequenceCurveDisplay", 0, 0, ImGui::GetContentRegionAvail(), plotFlags)) {
		
		//draw manoeuvre bounds
		if (getType() != ManoeuvreType::KEY) {
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
		
		//draw animation curves
		for (auto& animation : getAnimations()) animation->drawCurves();
		
		if(!Project::isPlotEditLocked()){
			if(auto selectedAnimation = getSelectedEditorAnimation()) selectedAnimation->drawCurveControls();
		}
        
        /*
		//draw curve editor controls
		for (auto& animation : getAnimations()) {
			ImGui::PushID(animation->getAnimatable()->getMachine()->getName());
			ImGui::PushID(animation->getAnimatable()->getName());
			animation->drawCurveControls();
			ImGui::PopID();
			ImGui::PopID();
		}
        */
		
		if(getType() != ManoeuvreType::KEY){
			double playbackTime = getSychronizedPlaybackPosition();
            
            if(Environnement::isSimulating()){
				if(ImPlot::DragLineX("Playhead", &playbackTime)) setSynchronizedPlaybackPosition(playbackTime);
            }else{
                if(!isnan(playbackTime)){
                    ImPlot::SetNextLineStyle(Colors::white, ImGui::GetTextLineHeight() * 0.1);
                    ImPlot::PlotVLines("Playhead", &playbackTime, 1);
                }
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
		

		if(ImPlot::IsPlotHovered()){
		
			if(ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !Project::isPlotEditLocked()){
				auto selectedCurve = getSelectedEditorCurve();
				auto selectedAnimation = getSelectedEditorAnimation();
				if(selectedCurve != nullptr && selectedAnimation != nullptr){
					ImPlotPoint mousePoint = ImPlot::GetPlotMousePos();
					selectedAnimation->addCurvePoint(selectedCurve, mousePoint.x, mousePoint.y);
				}
			}else if(ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right)) requestCurveRefocus();
			else if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
				clearControlPointSelection();
			}
		}
		
		if(!Project::isPlotEditLocked() && ImGui::IsWindowFocused() && (ImGui::IsKeyPressed(GLFW_KEY_DELETE) || ImGui::IsKeyPressed(GLFW_KEY_BACKSPACE))){
			deletedControlPointSelection();
			selectedEditorCurve->refresh();
		}
		
		ImPlot::EndPlot();
	}
	
	
	
}
