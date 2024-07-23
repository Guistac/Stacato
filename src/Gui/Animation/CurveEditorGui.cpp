#include <pch.h>

#include <imgui.h>
#include <implot.h>
#include <GLFW/glfw3.h>

#include "Animation/Manoeuvre.h"
#include "Animation/Animation.h"
#include "Animation/Animatable.h"
#include "Machine/Machine.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Stacato/StacatoEditor.h"
#include "Stacato/Project/StacatoProject.h"

void Manoeuvre::curveEditor(){
	
	auto currentProject = Stacato::Editor::getCurrentProject();
	
	//——————————— Curve List and point editor ——————————————
	
	float sequenceEditorWidth = ImGui::GetTextLineHeight() * 12.0;
	float sequenceEditorHeight = ImGui::GetContentRegionAvail().y;
	ImGui::BeginChild("SequenceEditor", glm::vec2(sequenceEditorWidth, sequenceEditorHeight));
	
	if(!currentProject->isPlotEditLocked()){
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetStyle().ItemSpacing.y));
		float inputFieldWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * .5f;
		
		glm::vec2 fieldsize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		ImColor backgroundColor = ImColor(Colors::veryDarkGray);
		float rounding = ImGui::GetStyle().FrameRounding;
		
		auto pointFieldEditor = [&](const char* name, double* data, const char* txt = "", const char* format = "%.3f") -> bool {
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
				return ImGui::InputDouble(name, data, 0.0, 0.0, format);
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
			pointFieldEditor("Ramp-In", nullptr, noSelectionString);
			pointFieldEditor("Ramp-Out", nullptr, noSelectionString);
		}else if(selectedControlPoints.size() > 1){
			static const char* multipleSelectionString = "Multiple";
			pointFieldEditor("Time", nullptr, multipleSelectionString);
			pointFieldEditor("Position", nullptr, multipleSelectionString);
			pointFieldEditor("Velocity", nullptr, multipleSelectionString);
			pointFieldEditor("Ramp-In", nullptr, multipleSelectionString);
			pointFieldEditor("Ramp-Out", nullptr, multipleSelectionString);
		}else{
			//single selection
			auto controlPoint = selectedControlPoints.front();
			bool b_pointEdited = false;
			if(pointFieldEditor("Time", &controlPoint->time, nullptr, "%.1fs")) b_pointEdited = true;
			
			char positionFormat[16];
			char velocityFormat[16];
			char accelerationFormat[16];
			snprintf(positionFormat, 16, "%s%s", "%.3f", controlPoint->unit->abbreviated);
			snprintf(velocityFormat, 16, "%s%s/s", "%.2f", controlPoint->unit->abbreviated);
			snprintf(accelerationFormat, 16, "%s%s/s\xc2\xb2", "%.2f", controlPoint->unit->abbreviated);
			if(pointFieldEditor("Position", &controlPoint->position, nullptr, positionFormat)) b_pointEdited = true;
			
			//TURN DISPLAY HACK
			if(controlPoint->unit == Units::AngularDistance::Degree){
				glm::vec2 cursor = ImGui::GetCursorPos();
				ImGui::Dummy(fieldsize);
				drawing->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), backgroundColor, rounding, ImDrawFlags_RoundCornersAll);
				ImGui::SetCursorPosX(cursor.x + ImGui::GetStyle().FramePadding.x);
				ImGui::SetCursorPosY(cursor.y + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) * .5f);
				ImGui::Text("Turns");
				ImGui::SameLine();
				ImGui::SetCursorPosY(cursor.y);
				ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 2.5);
				bool moduloEdited = false;
				moduloEdited |= ImGui::InputScalar("##turnCount", ImGuiDataType_S32, &controlPoint->turnCount, nullptr, nullptr, "%iR");
				ImGui::SameLine(0.0, 1.0);
				ImGui::SetCursorPosY(cursor.y);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				moduloEdited |=  ImGui::InputDouble("##degrees", &controlPoint->singleturndegrees, 0, 0, "+%.3f°");
				if(moduloEdited){
					controlPoint->singleturndegrees = std::clamp(controlPoint->singleturndegrees, 0.0, 359.999);
					controlPoint->position = controlPoint->turnCount * 360.0 + controlPoint->singleturndegrees;
					b_pointEdited = true;
				}
			}
			
			if(pointFieldEditor("Velocity", &controlPoint->velocity, nullptr, velocityFormat)) b_pointEdited = true;
			if(pointFieldEditor("Ramp-In", &controlPoint->inAcceleration, nullptr, accelerationFormat)) b_pointEdited = true;
			if(pointFieldEditor("Ramp-Out", &controlPoint->outAcceleration, nullptr, accelerationFormat)) b_pointEdited = true;
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
			
			ImGui::SetNextItemAllowOverlap();
			ImGui::InvisibleButton("background", glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()));
			glm::vec2 min = ImGui::GetItemRectMin();
			glm::vec2 max = ImGui::GetItemRectMax();
			
			ImGui::SetCursorPosY(lineStartCursor.y);
			ImGui::SetCursorPosX(lineStartCursor.x + ImGui::GetFrameHeight());
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
	
	static double speedFactor = 1.0;
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
	ImGui::InputDouble("##speedFactor", &speedFactor);
	ImGui::SameLine();
	if(ImGui::Button("Change Global Speed")){
		for(auto animation : getAnimations()){
			if(auto sequence = std::dynamic_pointer_cast<SequenceAnimation>(animation)){
				sequence->changeGlobalSpeed(speedFactor);
			}
		}
	}
	
	static double from = 0.0;
	static double amount = 0.0;
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
	ImGui::InputDouble("##from", &from);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
	ImGui::InputDouble("##amount", &amount);
	ImGui::SameLine();
	if(ImGui::Button("Subtract")){
		for(auto animation : getAnimations()){
			if(auto sequence = std::dynamic_pointer_cast<SequenceAnimation>(animation)){
				sequence->subtractTime(from, amount);
			}
		}
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
		ImPlot::SetNextAxesLimits(minX, maxX, minY, maxY, ImGuiCond_Always);
	}
	
	ImPlotFlags plotFlags = ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoLegend;
	
	ImPlot::GetStyle().Use24HourClock = true;
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
				ImPlot::PlotShaded("##shaded", &limits.front().x, &limits.front().y, limits.size(), -INFINITY, ImPlotShadedFlags_None, 0, sizeof(glm::vec2));
				ImPlot::PlotInfLines("##Limits1", &startTime, 1, ImPlotInfLinesFlags_None);
				ImPlot::PlotInfLines("##Limits2", &endTime, 1, ImPlotInfLinesFlags_None);
			}
		}
		
		//draw animation curves
		for (auto& animation : getAnimations()) animation->drawCurves();
		
		if(!currentProject->isPlotEditLocked()){
			if(auto selectedAnimation = getSelectedEditorAnimation()) selectedAnimation->drawCurveControls();
		}
        
		//draw curve editor controls
		for (auto& animation : getAnimations()) {
			ImGui::PushID(animation->getAnimatable()->getMachine()->getName());
			ImGui::PushID(animation->getAnimatable()->getName());
			animation->drawCurveControls();
			ImGui::PopID();
			ImGui::PopID();
		}
		
		if(getType() != ManoeuvreType::KEY){
			double playbackTime = getSychronizedPlaybackPosition();
            
            if(Environnement::isSimulating()){
				if(ImPlot::DragLineX(33333, &playbackTime, Colors::white)) setSynchronizedPlaybackPosition(playbackTime);
            }else{
                if(!isnan(playbackTime)){
                    ImPlot::SetNextLineStyle(Colors::white, ImGui::GetTextLineHeight() * 0.1);
					ImPlot::PlotInfLines("Playhead", &playbackTime, 1, ImPlotInfLinesFlags_None);
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
		
			if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsKeyDown(ImGuiKey_LeftShift)){
				auto selectedCurve = getSelectedEditorCurve();
				auto selectedAnimation = getSelectedEditorAnimation();
				if(selectedCurve != nullptr && selectedAnimation != nullptr){
					ImPlotPoint mousePoint = ImPlot::GetPlotMousePos();
					selectedAnimation->addCurvePoint(selectedCurve, mousePoint.x, mousePoint.y);
				}
			}
			
			//double clicking causes refocus
			else if(ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !currentProject->isPlotEditLocked()){
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
		
		if(!currentProject->isPlotEditLocked() && ImGui::IsWindowFocused() && (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace))){
			deletedControlPointSelection();
			selectedEditorCurve->refresh();
		}
		
		ImPlot::EndPlot();
	}
	
	
	
}
