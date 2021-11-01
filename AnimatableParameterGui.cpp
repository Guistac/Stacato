#include <pch.h>

#include "Motion/Machine/AnimatableParameter.h"

#include <imgui.h>
#include <imgui_internal.h>

void AnimatableParameterValue::inputFieldGui() {
	switch (type) {
		case ParameterDataType::BOOLEAN_PARAMETER:
			ImGui::Checkbox("##parameter", &boolValue);
			break;
		case ParameterDataType::INTEGER_PARAMETER:
			ImGui::InputInt("##parameter", &integerValue);
			break;
		case ParameterDataType::STATE_PARAMETER:
			if (ImGui::BeginCombo("##parameter", stateValue->displayName)) {
				for (auto& val : *stateValues) {
					if (ImGui::Selectable(val.displayName, stateValue == &val)) {
						stateValue = &val;
						break;
					}
				}
				ImGui::EndCombo();
			}
			break;
		case ParameterDataType::REAL_PARAMETER:
		case ParameterDataType::KINEMATIC_POSITION_CURVE:
			ImGui::InputDouble("##parameter", &realValue);
			break;
		case ParameterDataType::VECTOR_2D_PARAMETER:
		case ParameterDataType::KINEMATIC_2D_POSITION_CURVE:
			ImGui::InputFloat2("##parameter", &vector2value.x);
			break;
		case ParameterDataType::VECTOR_3D_PARAMETER:
		case ParameterDataType::KINEMATIC_3D_POSITION_CURVE:
			ImGui::InputFloat3("##parameter", &vector3value.x);
			break;
	}
}