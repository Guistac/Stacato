#include <pch.h>

#include "Fieldbus/Utilities/EtherCatData.h"

#include <imgui.h>
#include <imgui_internal.h>

void EtherCatData::valueTextGui() {
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	switch (dataType) {
		case EtherCatDataType::Type::UINT8_T:	ImGui::InputScalar("##input", ImGuiDataType_U8, &u8); break;
		case EtherCatDataType::Type::INT8_T:	ImGui::InputScalar("##input", ImGuiDataType_S8, &s8); break;
		case EtherCatDataType::Type::UINT16_T:	ImGui::InputScalar("##input", ImGuiDataType_U16, &u16); break;
		case EtherCatDataType::Type::INT16_T:	ImGui::InputScalar("##input", ImGuiDataType_S16, &s16); break;
		case EtherCatDataType::Type::UINT32_T:	ImGui::InputScalar("##input", ImGuiDataType_U32, &u32); break;
		case EtherCatDataType::Type::INT32_T:	ImGui::InputScalar("##input", ImGuiDataType_S32, &s32); break;
		case EtherCatDataType::Type::UINT64_T:	ImGui::InputScalar("##input", ImGuiDataType_U64, &u64); break;
		case EtherCatDataType::Type::INT64_T:	ImGui::InputScalar("##input", ImGuiDataType_S64, &s64); break;
	}
	ImGui::PopItemFlag();
}

bool EtherCatData::dataEditFieldGui() {
	switch (dataType) {
		case EtherCatDataType::Type::UINT8_T:	return ImGui::InputScalar("##input", ImGuiDataType_U8, &u8); break;
		case EtherCatDataType::Type::INT8_T:	return ImGui::InputScalar("##input", ImGuiDataType_S8, &s8); break;
		case EtherCatDataType::Type::UINT16_T:	return ImGui::InputScalar("##input", ImGuiDataType_U16, &u16); break;
		case EtherCatDataType::Type::INT16_T:	return ImGui::InputScalar("##input", ImGuiDataType_S16, &s16); break;
		case EtherCatDataType::Type::UINT32_T:	return ImGui::InputScalar("##input", ImGuiDataType_U32, &u32); break;
		case EtherCatDataType::Type::INT32_T:	return ImGui::InputScalar("##input", ImGuiDataType_S32, &s32); break;
		case EtherCatDataType::Type::UINT64_T:	return ImGui::InputScalar("##input", ImGuiDataType_U64, &u64); break;
		case EtherCatDataType::Type::INT64_T:	return ImGui::InputScalar("##input", ImGuiDataType_S64, &s64); break;
	}
	return false;
}

bool EtherCatData::dataTypeSelectorGui() {
	bool newTypeSelected = false;
	if (ImGui::BeginCombo("##DataTypeSelector", getTypeString())) {
		for (EtherCatDataType& type : dataTypes) {
			if (ImGui::Selectable(type.displayName, type.type == dataType)) {
				dataType = type.type;
				newTypeSelected = true;
			}
		}
		ImGui::EndCombo();
	}
	return newTypeSelected;
}

bool EtherCatData::indexEditFieldGui() {
	static char indexInput[5];
	sprintf(indexInput, "%X", index);
	if (ImGui::InputText("##Index", indexInput, 5, ImGuiInputTextFlags_CharsHexadecimal)) {
		if (strcmp(indexInput, "") == 0) index = 0;
		std::stringstream hexStream;
		hexStream << std::hex << indexInput;
		hexStream >> index;
		return true;
	}
	return false;
}

bool EtherCatData::subindexEditFieldGui() {
	//for some reason this method doesn't accept uint8_t values
	//so we use a dummy uin16_t value and convert back to uint8_t after processing
	uint16_t subindex16 = subindex;
	static char subIndexInput[3];
	sprintf(subIndexInput, "%X", subindex16);
	if (ImGui::InputText("##SubIndex", subIndexInput, 3, ImGuiInputTextFlags_CharsHexadecimal)) {
		if (strcmp(subIndexInput, "") == 0) subindex16 = 0;
		std::stringstream hexStream;
		hexStream << std::hex << subIndexInput;
		hexStream >> subindex16;
		subindex = subindex16;
		return true;
	}
	return false;
}