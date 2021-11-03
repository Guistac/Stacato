#include <pch.h>

#include "Motion/AnimatableParameter.h"

#include <imgui.h>
#include <imgui_internal.h>

bool AnimatableParameterValue::inputFieldGui(float width) {
	bool valueChanged = false;
	ImGui::SetNextItemWidth(width);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.1));
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
			valueChanged = ImGui::InputDouble("##parameter", &realValue);
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			valueChanged |= ImGui::InputFloat("##X", &vector2value.x, 0.0, 0.0, "X: %.3f");
			ImGui::SetNextItemWidth(width);
			valueChanged |= ImGui::InputFloat("##Y", &vector2value.y, 0.0, 0.0, "Y: %.3f");
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			valueChanged |= ImGui::InputFloat("##X", &vector3value.x, 0.0, 0.0, "X: %.3f");
			ImGui::SetNextItemWidth(width);
			valueChanged |= ImGui::InputFloat("##Y", &vector3value.y, 0.0, 0.0, "Y: %.3f");
			ImGui::SetNextItemWidth(width);
			valueChanged |= ImGui::InputFloat("##Z", &vector3value.z, 0.0, 0.0, "Z: %.3f");
			break;
	}
	ImGui::PopStyleVar();
	return valueChanged;
}