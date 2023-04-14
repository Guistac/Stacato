#include <pch.h>

#include "StringParameter.h"
#include "NumberParameter.h"
#include "BooleanParameter.h"
#include "VectorParameter.h"
#include "OptionParameter.h"
#include "TimeParameter.h"

#include <imgui.h>

namespace Legato{
	
	void StringParameter::onGui() {
		ImGui::BeginDisabled(isDisabled());
		ImGui::InputText(getImGuiID(), displayValue, bufferSize);
		ImGui::EndDisabled();
		if(ImGui::IsItemDeactivatedAfterEdit()){
			overwriteWithHistory(displayValue);
		}
	}

	void BooleanParameter::onGui() {
		ImGui::BeginDisabled(isDisabled());
		ImGui::Checkbox(getImGuiID(), &displayValue);
		ImGui::EndDisabled();
		if(ImGui::IsItemDeactivatedAfterEdit()){
			overwriteWithHistory(displayValue);
		}
	}

	template<typename T>
	void VectorParameter<T>::onGui() {
		inputField();
		if(ImGui::IsItemDeactivatedAfterEdit()){
			overwriteWithHistory(displayValue);
		}
	}

	template<>
	inline void VectorParameter<glm::vec2>::inputField(){
		ImGui::BeginDisabled(isDisabled());
		//ImGui::InputFloat2(getImGuiID(), &displayValue.x, format);
		ImGui::EndDisabled();
	}

	template<>
	inline bool VectorParameter<glm::vec2>::save(){
		bool success = true;
		success &= serializeAttribute("X", value.x);
		success &= serializeAttribute("Y", value.y);
		return success;
	}

	template<>
	inline bool VectorParameter<glm::vec2>::load(){
		bool success = true;
		success &= deserializeAttribute("X", value.x);
		success &= deserializeAttribute("Y", value.y);
		overwrite(value);
		return success;
	}


	template<>
	inline void VectorParameter<glm::ivec2>::inputField(){
		ImGui::BeginDisabled(isDisabled());
		ImGui::InputInt2(getImGuiID(), &displayValue.x);
		ImGui::EndDisabled();
	}

	template<>
	inline bool VectorParameter<glm::ivec2>::save(){
		bool success = true;
		success &= serializeAttribute("X", value.x);
		success &= serializeAttribute("Y", value.y);
		return success;
	}

	template<>
	inline bool VectorParameter<glm::ivec2>::load(){
		bool success = true;
		success &= deserializeAttribute("X", value.x);
		success &= deserializeAttribute("Y", value.y);
		overwrite(value);
		return success;
	}


	template<>
	inline void VectorParameter<glm::vec3>::inputField(){
		ImGui::BeginDisabled(isDisabled());
		ImGui::InputFloat3(getImGuiID(), &displayValue.x, format.c_str());
		ImGui::EndDisabled();
	}

	template<>
	inline bool VectorParameter<glm::vec3>::save(){
		bool success = true;
		success &= serializeAttribute("X", value.x);
		success &= serializeAttribute("Y", value.y);
		success &= serializeAttribute("Z", value.y);
		return success;
	}

	template<>
	inline bool VectorParameter<glm::vec3>::load(){
		bool success = true;
		success &= deserializeAttribute("X", value.x);
		success &= deserializeAttribute("Y", value.y);
		success &= deserializeAttribute("Z", value.y);
		overwrite(value);
		return success;
	}


	template<>
	inline void VectorParameter<glm::vec4>::inputField(){
		ImGui::BeginDisabled(isDisabled());
		ImGui::InputFloat4(getImGuiID(), &displayValue.x, format.c_str());
		ImGui::EndDisabled();
	}

	template<>
	inline bool VectorParameter<glm::vec4>::save(){
		bool success = true;
		success &= serializeAttribute("X", value.x);
		success &= serializeAttribute("Y", value.y);
		success &= serializeAttribute("Z", value.y);
		success &= serializeAttribute("W", value.y);
		return success;
	}

	template<>
	inline bool VectorParameter<glm::vec4>::load(){
		bool success = true;
		success &= deserializeAttribute("X", value.x);
		success &= deserializeAttribute("Y", value.y);
		success &= deserializeAttribute("Z", value.y);
		success &= deserializeAttribute("W", value.y);
		overwrite(value);
		return success;
	}

	template class VectorParameter<glm::vec2>;
	template class VectorParameter<glm::ivec2>;
	template class VectorParameter<glm::vec3>;
	template class VectorParameter<glm::vec4>;







	void OptionParameter::onGui() {
		ImGui::BeginDisabled(isDisabled());
		const char* previewString = "";
		if(displayValue) previewString = displayValue->displayString.c_str();
		if(ImGui::BeginCombo(getImGuiID(), previewString)){
			for(auto& option : optionList){
				ImGui::BeginDisabled(!option->b_enabled);
				if(ImGui::Selectable(option->displayString.c_str(), value == option->enumerator)){
					overwriteWithHistory(*option);
				}
				ImGui::EndDisabled();
			}
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();
	}


	void TimeParameter::onGui() {
		ImGui::BeginDisabled(isDisabled());
		ImGui::InputText(getImGuiID(), displayBuffer, 32, ImGuiInputTextFlags_AutoSelectAll);
		if(ImGui::IsItemDeactivatedAfterEdit()){
			//double newTime = TimeStringConversion::timecodeStringToSeconds(displayBuffer);
			//overwriteWithHistory(newTime);
		}
		ImGui::EndDisabled();
	}
	
};
