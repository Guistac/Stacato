#include "ATV320.h"

#include <imgui.h>

void ATV320::deviceSpecificGui(){
	
	if(ImGui::BeginTabItem("Controls")){
	
		if(ImGui::BeginCombo("Requested Power State", Enumerator::getDisplayString(powerStateTarget))){
			for(auto& state : Enumerator::getTypes<DS402::PowerState>()){
				if(ImGui::Selectable(state.displayString, state.enumerator == powerStateTarget)){
					powerStateTarget = state.enumerator;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::Text("Actual Power State: %s", Enumerator::getDisplayString(powerStateActual));
		
		if(ImGui::Button("Reset Fault")) b_resetFaultCommand = true;
		
		ImGui::InputFloat("AccelerationTime", &accelerationTime_seconds, 0.0, 0.0, "%.2f s");
		ImGui::InputFloat("DecelerationTime", &decelerationTime_seconds, 0.0, 0.0, "%.2f s");
		
		ImGui::Separator();
		
		ImGui::Text("Velocity Control");
		
		int maxVelocity = 2000;
		int16_t max = maxVelocity;
		int16_t min = -maxVelocity;
		ImGui::SliderScalar("##VelocityTarget", ImGuiDataType_S16, &velocityTarget, &min, &max);
		if(ImGui::IsItemDeactivatedAfterEdit()) velocityTarget = 0;
		
		ImGui::InvisibleButton("velocityIndicator", ImGui::GetItemRectSize());
		glm::vec2 minIndicator = ImGui::GetItemRectMin();
		glm::vec2 maxIndicator = ImGui::GetItemRectMax();
		glm::vec2 sizeIndicator = ImGui::GetItemRectSize();
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		drawing->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::GetColorU32(ImGuiCol_Button), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		float velocityNormalized = (float)velocityActual / maxVelocity;
		float sizeIndicatorWidthHalf = sizeIndicator.x / 2.0;
		float velocityWidth = velocityNormalized * sizeIndicatorWidthHalf;
		if(velocityNormalized > 0.0f){
			glm::vec2 start(minIndicator.x + sizeIndicatorWidthHalf, minIndicator.y);
			glm::vec2 end(start.x + velocityWidth, maxIndicator.y);
			drawing->AddRectFilled(start, end, ImGui::GetColorU32(ImGuiCol_PlotHistogram), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		}else if(velocityNormalized < 0.0f){
			glm::vec2 start(minIndicator.x + sizeIndicatorWidthHalf + velocityWidth, minIndicator.y);
			glm::vec2 end(minIndicator.x + sizeIndicatorWidthHalf, maxIndicator.y);
			drawing->AddRectFilled(start, end, ImGui::GetColorU32(ImGuiCol_PlotHistogram), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
		}
		glm::vec2 minCenter = glm::vec2(minIndicator.x + sizeIndicatorWidthHalf, minIndicator.y);
		glm::vec2 maxCenter = glm::vec2(minIndicator.x + sizeIndicatorWidthHalf, maxIndicator.y);
		ImColor centerColor = b_velocityTargetReached ? ImColor(.0f, 1.0f, .0f, 1.f) : ImColor(1.f, 1.f, 1.f, 1.f);
		drawing->AddRectFilled(minCenter, maxCenter, centerColor, 2.0);
		ImGui::Text("Velocity: %i", velocityActual);
		
		std::bitset<16> bits16;
		bits16 = logicInputs;
		ImGui::Text("Logic Inputs: %s", bits16.to_string().c_str());
		
		ImGui::EndTabItem();
	}
	
}
