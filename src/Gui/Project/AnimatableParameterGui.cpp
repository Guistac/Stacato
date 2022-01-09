#include <pch.h>

#include "Motion/AnimatableParameter.h"

#include <imgui.h>
#include <imgui_internal.h>

bool AnimatableParameterValue::inputFieldGui(float width) {
	static char inputFieldString[128];
	bool valueChanged = false;
	ImGui::SetNextItemWidth(width);
	switch (type) {
		case ParameterDataType::Type::BOOLEAN_PARAMETER:
			valueChanged = ImGui::Checkbox("##parameter", &boolValue);
			break;
		case ParameterDataType::Type::INTEGER_PARAMETER:
			valueChanged = ImGui::InputInt("##parameter", &integerValue);
			break;
		case ParameterDataType::Type::STATE_PARAMETER:
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
		case ParameterDataType::Type::REAL_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_POSITION_CURVE:
			sprintf(inputFieldString, "%.1f %s", realValue, shortUnitString);
			valueChanged = ImGui::InputDouble("##parameter", &realValue, 0.0, 0.0, inputFieldString);
			break;
		case ParameterDataType::Type::VECTOR_2D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_2D_POSITION_CURVE:
			sprintf(inputFieldString, "x: %.1f %s", vector2value.x, shortUnitString);
			valueChanged |= ImGui::InputFloat("##X", &vector2value.x, 0.0, 0.0, inputFieldString);
			ImGui::SetNextItemWidth(width);
			sprintf(inputFieldString, "y: %.1f %s", vector2value.y, shortUnitString);
			valueChanged |= ImGui::InputFloat("##Y", &vector2value.y, 0.0, 0.0, inputFieldString);
			break;
		case ParameterDataType::Type::VECTOR_3D_PARAMETER:
		case ParameterDataType::Type::KINEMATIC_3D_POSITION_CURVE:
			sprintf(inputFieldString, "x: %.1f %s", vector3value.x, shortUnitString);
			valueChanged |= ImGui::InputFloat("##X", &vector3value.x, 0.0, 0.0, inputFieldString);
			ImGui::SetNextItemWidth(width);
			sprintf(inputFieldString, "y: %.1f %s", vector3value.y, shortUnitString);
			valueChanged |= ImGui::InputFloat("##Y", &vector3value.y, 0.0, 0.0, inputFieldString);
			ImGui::SetNextItemWidth(width);
			sprintf(inputFieldString, "z: %.1f %s", vector3value.z, shortUnitString);
			valueChanged |= ImGui::InputFloat("##Z", &vector3value.z, 0.0, 0.0, inputFieldString);
			break;
		case ParameterDataType::Type::PARAMETER_GROUP:
			break;
	}
	return valueChanged;
}
