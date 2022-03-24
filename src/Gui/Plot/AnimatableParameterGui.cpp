#include <pch.h>

#include "Machine/AnimatableParameter.h"

#include <imgui.h>
#include <GLFW/glfw3.h>

bool AnimatableParameterValue::inputFieldGui(float width) {
	static char inputFieldString[128];
	bool valueChanged = false;
	ImGui::SetNextItemWidth(width);
	switch (type) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			valueChanged = ImGui::Checkbox("##parameter", &boolValue);
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			valueChanged = ImGui::InputInt("##parameter", &integerValue);
			break;
		case ParameterDataType::STATE_PARAMETER:
			if (ImGui::BeginCombo("##parameter", stateValue->displayName)) {
				for (auto& val : *stateValues) {
					if (ImGui::Selectable(val.displayName, stateValue == &val)) {
						stateValue = &val;
						valueChanged = true;
						break;
					}
				}
				ImGui::EndCombo();
			}
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
			sprintf(inputFieldString, "%.3f %s", realValue, shortUnitString);
			valueChanged = ImGui::InputDouble("##parameter", &realValue, 0.0, 0.0, inputFieldString);
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			sprintf(inputFieldString, "x: %.3f %s", vector2value.x, shortUnitString);
			valueChanged |= ImGui::InputFloat("##X", &vector2value.x, 0.0, 0.0, inputFieldString);
			ImGui::SetNextItemWidth(width);
			sprintf(inputFieldString, "y: %.3f %s", vector2value.y, shortUnitString);
			valueChanged |= ImGui::InputFloat("##Y", &vector2value.y, 0.0, 0.0, inputFieldString);
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			sprintf(inputFieldString, "x: %.3f %s", vector3value.x, shortUnitString);
			valueChanged |= ImGui::InputFloat("##X", &vector3value.x, 0.0, 0.0, inputFieldString);
			ImGui::SetNextItemWidth(width);
			sprintf(inputFieldString, "y: %.3f %s", vector3value.y, shortUnitString);
			valueChanged |= ImGui::InputFloat("##Y", &vector3value.y, 0.0, 0.0, inputFieldString);
			ImGui::SetNextItemWidth(width);
			sprintf(inputFieldString, "z: %.3f %s", vector3value.z, shortUnitString);
			valueChanged |= ImGui::InputFloat("##Z", &vector3value.z, 0.0, 0.0, inputFieldString);
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
		switch (type) {
			case ParameterDataType::REAL_PARAMETER:
			case ParameterDataType::KINEMATIC_POSITION_CURVE:
				ImGui::Text("Type : Double");
				ImGui::Text("Value : %.20f", realValue);
				break;
			case ParameterDataType::VECTOR_2D_PARAMETER:
			case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
				ImGui::Text("Type : 2D Double");
				ImGui::Text("Value X : %.20f", vector2value.x);
				ImGui::Text("Value Y : %.20f", vector2value.y);
				break;
			case ParameterDataType::VECTOR_3D_PARAMETER:
			case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
				ImGui::Text("Type : 3D Double");
				ImGui::Text("Value X : %.20f", vector3value.x);
				ImGui::Text("Value Y : %.20f", vector3value.y);
				ImGui::Text("Value Z : %.20f", vector3value.z);
				break;
			default: break;
		}
		ImGui::EndTooltip();
	}
	
	return valueChanged;
}
