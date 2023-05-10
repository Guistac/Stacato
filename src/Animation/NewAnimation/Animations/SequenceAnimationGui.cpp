#include <pch.h>

#include "SequenceAnimation.h"

namespace AnimationSystem{

	void SequenceAnimation::parameterGui(){
		
		auto drawParameterGroup = [](std::vector<std::shared_ptr<Legato::Parameter>> parameters){
				
			if(ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_RowBg |ImGuiTableFlags_SizingFixedSame)){
				
				ImGui::TableSetupColumn("Parameter");
				ImGui::TableSetupColumn("Value");
				
				float frameHeight = ImGui::GetFrameHeight();
				ImVec2 offset(ImGui::GetStyle().CellPadding.y, (frameHeight - ImGui::GetTextLineHeight()) / 2.0);
				
				for(auto parameter : parameters){
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImVec2 cursorPos = ImGui::GetCursorPos();
					ImGui::SetCursorPos(ImVec2(cursorPos.x + offset.x, cursorPos.y + offset.y));
					
					ImGui::BeginDisabled(parameter->isDisabled());
					ImGui::Text("%s", parameter->getName().c_str());
					ImGui::EndDisabled();
					
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().CellPadding.y);
					parameter->gui();
				}
				
				ImGui::EndTable();
			}
		};
		
		drawParameterGroup({
			curveStartParameter,
			curveTargetParameter,
			curveDeltaParameter,
			rampInParameter,
			rampOutParameter,
			curveDurationParameter
		});
	}

};
