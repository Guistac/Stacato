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
	void NumberParameter<T>::onGui() {
		ImGui::BeginDisabled(isDisabled());
		ImGui::InputScalar(getImGuiID(), getImGuiDataType(), &displayValue, stepSmallPtr, stepLargePtr, getFormatString(), ImGuiInputTextFlags_CharsScientific);
		ImGui::EndDisabled();
		if(ImGui::IsItemDeactivatedAfterEdit()){
			overwriteWithHistory(displayValue);
		}
	}

	template<>
	inline ImGuiDataType NumberParameter<float>::getImGuiDataType(){ return ImGuiDataType_Float; }
	template<>
	inline ImGuiDataType NumberParameter<double>::getImGuiDataType(){ return ImGuiDataType_Double; }
	template<>
	inline ImGuiDataType NumberParameter<uint8_t>::getImGuiDataType(){ return ImGuiDataType_U8; }
	template<>
	inline ImGuiDataType NumberParameter<int8_t>::getImGuiDataType(){ return ImGuiDataType_S8; }
	template<>
	inline ImGuiDataType NumberParameter<uint16_t>::getImGuiDataType(){ return ImGuiDataType_U16; }
	template<>
	inline ImGuiDataType NumberParameter<int16_t>::getImGuiDataType(){ return ImGuiDataType_S16; }
	template<>
	inline ImGuiDataType NumberParameter<uint32_t>::getImGuiDataType(){ return ImGuiDataType_U32; }
	template<>
	inline ImGuiDataType NumberParameter<int32_t>::getImGuiDataType(){ return ImGuiDataType_S32; }
	template<>
	inline ImGuiDataType NumberParameter<uint64_t>::getImGuiDataType(){ return ImGuiDataType_U64; }
	template<>
	inline ImGuiDataType NumberParameter<int64_t>::getImGuiDataType(){ return ImGuiDataType_S64; }

	template<>
	inline std::string NumberParameter<float>::getDefaultFormatString(){ return "%.3f"; }
	template<>
	inline std::string NumberParameter<double>::getDefaultFormatString(){ return "%.3f"; }


	template class NumberParameter<float>;
	template class NumberParameter<double>;
	template class NumberParameter<uint8_t>;
	template class NumberParameter<int8_t>;
	template class NumberParameter<uint16_t>;
	template class NumberParameter<int16_t>;
	template class NumberParameter<uint32_t>;
	template class NumberParameter<int32_t>;
	template class NumberParameter<uint64_t>;
	template class NumberParameter<int64_t>;


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
