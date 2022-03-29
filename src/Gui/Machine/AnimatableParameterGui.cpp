#include <pch.h>

#include "Machine/AnimatableParameter.h"

#include <imgui.h>
#include <GLFW/glfw3.h>

bool AnimatableParameterValue::inputFieldGui(float width) {
	static char inputFieldString[128];
	bool valueChanged = false;
	ImGui::SetNextItemWidth(width);
	switch (parameter->dataType) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			valueChanged = ImGui::Checkbox("##parameter", &boolean);
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			valueChanged = ImGui::InputInt("##parameter", &integer);
			break;
		case ParameterDataType::STATE_PARAMETER:
			if (ImGui::BeginCombo("##parameter", state->displayName)) {
				for (auto& val : parameter->getStateValues()) {
					if (ImGui::Selectable(val.displayName, state == &val)) {
						state = &val;
						valueChanged = true;
						break;
					}
				}
				ImGui::EndCombo();
			}
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::POSITION:
			sprintf(inputFieldString, "%.3f %s", real, parameter->unit->abbreviated);
			valueChanged = ImGui::InputDouble("##parameter", &real, 0.0, 0.0, inputFieldString);
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::POSITION_2D:
			sprintf(inputFieldString, "x: %.3f %s", vector2.x, parameter->unit->abbreviated);
			valueChanged |= ImGui::InputFloat("##X", &vector2.x, 0.0, 0.0, inputFieldString);
			ImGui::SetNextItemWidth(width);
			sprintf(inputFieldString, "y: %.3f %s", vector2.y, parameter->unit->abbreviated);
			valueChanged |= ImGui::InputFloat("##Y", &vector2.y, 0.0, 0.0, inputFieldString);
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::POSITION_3D:
			sprintf(inputFieldString, "x: %.3f %s", vector3.x, parameter->unit->abbreviated);
			valueChanged |= ImGui::InputFloat("##X", &vector3.x, 0.0, 0.0, inputFieldString);
			ImGui::SetNextItemWidth(width);
			sprintf(inputFieldString, "y: %.3f %s", vector3.y, parameter->unit->abbreviated);
			valueChanged |= ImGui::InputFloat("##Y", &vector3.y, 0.0, 0.0, inputFieldString);
			ImGui::SetNextItemWidth(width);
			sprintf(inputFieldString, "z: %.3f %s", vector3.z, parameter->unit->abbreviated);
			valueChanged |= ImGui::InputFloat("##Z", &vector3.z, 0.0, 0.0, inputFieldString);
			break;
		case ParameterDataType::PARAMETER_GROUP:
			break;
	}

	//hacky way of getting hover status regardless of ItemFlags_Disable
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	glm::vec2 mouse = ImGui::GetIO().MousePos;
	bool b_hovered = mouse.x > min.x && mouse.x < max.x && mouse.y > min.y && mouse.y < max.y;
	
	if(b_hovered && ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL)){
		ImGui::BeginTooltip();
		switch (parameter->dataType) {
			case ParameterDataType::REAL_PARAMETER:
			case ParameterDataType::POSITION:
				ImGui::Text("Type : Double");
				ImGui::Text("Value : %.20f", real);
				break;
			case ParameterDataType::VECTOR_2D_PARAMETER:
			case ParameterDataType::POSITION_2D:
				ImGui::Text("Type : 2D Double");
				ImGui::Text("Value X : %.20f", vector2.x);
				ImGui::Text("Value Y : %.20f", vector2.y);
				break;
			case ParameterDataType::VECTOR_3D_PARAMETER:
			case ParameterDataType::POSITION_3D:
				ImGui::Text("Type : 3D Double");
				ImGui::Text("Value X : %.20f", vector3.x);
				ImGui::Text("Value Y : %.20f", vector3.y);
				ImGui::Text("Value Z : %.20f", vector3.z);
				break;
			default: break;
		}
		ImGui::EndTooltip();
	}
	
	return valueChanged;
}
