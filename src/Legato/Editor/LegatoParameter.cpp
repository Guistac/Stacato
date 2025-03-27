#include "LegatoParameter.h"

#include <imgui.h>

void Legato::BoolParam::gui(bool b_drawName){
	ImGui::Checkbox(imguiID.c_str(), &displayValue);
	if(ImGui::IsItemDeactivatedAfterEdit()){
		value = displayValue;
	}
	if(b_drawName){
		ImGui::SameLine();
		ImGui::Text("%s", name.c_str());
	}
}

void Legato::IntParam::gui(bool b_drawName){
	if(b_drawName) ImGui::Text("%s", name.c_str());
	ImGui::InputScalar(imguiID.c_str(), ImGuiDataType_S64, &displayValue);
	if(ImGui::IsItemDeactivatedAfterEdit()){
		value = displayValue;
	}
}

void Legato::RealParam::gui(bool b_drawName){
	if(b_drawName) ImGui::Text("%s", name.c_str());
	ImGui::InputDouble(imguiID.c_str(), &displayValue);
	if(ImGui::IsItemDeactivatedAfterEdit()){
		value = displayValue;
	}
}

void Legato::StrParam::gui(bool b_drawName){
	if(b_drawName) ImGui::Text("%s", name.c_str());
	ImGui::InputText(imguiID.c_str(), displayValue, 256);
	if(ImGui::IsItemDeactivatedAfterEdit()){
		value = displayValue;
	}
}

void Legato::OptParam::gui(bool b_drawName){
	if(b_drawName) ImGui::Text("%s", name.c_str());
	std::string preview = "";
	if(displayValue) preview = displayValue->name;
	if(ImGui::BeginCombo(imguiID.c_str(), preview.c_str())){
		for(auto& option : options){
			ImGui::BeginDisabled(!option.b_enabled);
			if(ImGui::Selectable(option.name.c_str(), value == option.enumerator)){
				displayValue = &option;
				value = option.enumerator;
			}
			ImGui::EndDisabled();
		}
		ImGui::EndCombo();
	}
	if(ImGui::IsItemDeactivatedAfterEdit()){
		value = displayValue->enumerator;
	}
}
