#include <pch.h>

#include "Fieldbus/Utilities/EtherCatData.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Framework/Colors.h"

void EtherCatBaseData::valueTextGui() {
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	dataEditFieldGui();
	ImGui::PopItemFlag();
}

bool EtherCatBaseData::dataEditFieldGui() {
	bool wasEdited = false;
	uint64_t value;
	static char inputString[65];
	switch (dataFormat) {
		case DataFormat::Type::STRING:
			switch (dataType) {
				case EtherCatData::Type::UINT8_T:	return ImGui::InputScalar("##input", ImGuiDataType_U8, &u8, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT8_T:	return ImGui::InputScalar("##input", ImGuiDataType_S8, &s8, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT16_T:	return ImGui::InputScalar("##input", ImGuiDataType_U16, &u16, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT16_T:	return ImGui::InputScalar("##input", ImGuiDataType_S16, &s16, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT32_T:	return ImGui::InputScalar("##input", ImGuiDataType_U32, &u32, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT32_T:	return ImGui::InputScalar("##input", ImGuiDataType_S32, &s32, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT64_T:	return ImGui::InputScalar("##input", ImGuiDataType_U64, &u64, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT64_T:	return ImGui::InputScalar("##input", ImGuiDataType_S64, &s64, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::STRING:	return ImGui::InputText("##input", stringBuffer, stringBufferSize); break;
			}
		case DataFormat::Type::DECIMAL:
			switch (dataType) {
				case EtherCatData::Type::UINT8_T:	return ImGui::InputScalar("##input", ImGuiDataType_U8, &u8, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT8_T:	return ImGui::InputScalar("##input", ImGuiDataType_S8, &s8, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT16_T:	return ImGui::InputScalar("##input", ImGuiDataType_U16, &u16, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT16_T:	return ImGui::InputScalar("##input", ImGuiDataType_S16, &s16, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT32_T:	return ImGui::InputScalar("##input", ImGuiDataType_U32, &u32, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT32_T:	return ImGui::InputScalar("##input", ImGuiDataType_S32, &s32, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT64_T:	return ImGui::InputScalar("##input", ImGuiDataType_U64, &u64, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT64_T:	return ImGui::InputScalar("##input", ImGuiDataType_S64, &s64, 0, 0, 0, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::STRING:	return ImGui::InputText("##input", stringBuffer, stringBufferSize); break;
			}
			break;
		case DataFormat::Type::HEXADECIMAL:
			switch (dataType) {
				case EtherCatData::Type::UINT8_T:	value = u8; break;
				case EtherCatData::Type::INT8_T:	value = s8;  break;
				case EtherCatData::Type::UINT16_T:	value = u16; break;
				case EtherCatData::Type::INT16_T:	value = s16; break;
				case EtherCatData::Type::UINT32_T:	value = u32; break;
				case EtherCatData::Type::INT32_T:	value = s32; break;
				case EtherCatData::Type::UINT64_T:	value = u64; break;
				case EtherCatData::Type::INT64_T:	value = s64; break;
				case EtherCatData::Type::STRING:	return ImGui::InputText("##input", stringBuffer, stringBufferSize); break;
			}

			sprintf(inputString, "%X", (int)value);
			switch (dataType) {
				case EtherCatData::Type::UINT8_T:	wasEdited = ImGui::InputText("##inputHex", inputString, 3, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT8_T:	wasEdited = ImGui::InputText("##inputHex", inputString, 3, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT16_T:	wasEdited = ImGui::InputText("##inputHex", inputString, 5, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT16_T:	wasEdited = ImGui::InputText("##inputHex", inputString, 5, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT32_T:	wasEdited = ImGui::InputText("##inputHex", inputString, 9, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT32_T:	wasEdited = ImGui::InputText("##inputHex", inputString, 9, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT64_T:	wasEdited = ImGui::InputText("##inputHex", inputString, 17, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT64_T:	wasEdited = ImGui::InputText("##inputHex", inputString, 17, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll); break;
			}
			if (wasEdited) {
				if (strcmp(inputString, "") == 0) value = 0;
				std::stringstream hexStream;
				hexStream << std::hex << inputString;
				hexStream >> value;
				switch (dataType) {
					case EtherCatData::Type::UINT8_T:	u8 = value; break;
					case EtherCatData::Type::INT8_T:	s8 = value;  break;
					case EtherCatData::Type::UINT16_T:	u16 = value; break;
					case EtherCatData::Type::INT16_T:	s16 = value; break;
					case EtherCatData::Type::UINT32_T:	u32 = value; break;
					case EtherCatData::Type::INT32_T:	s32 = value; break;
					case EtherCatData::Type::UINT64_T:	u64 = value; break;
					case EtherCatData::Type::INT64_T:	s64 = value; break;
				}
				return true;
			}
			break;
		case DataFormat::Type::BINARY:
			std::bitset<8> bits8;
			std::bitset<16> bits16;
			std::bitset<32> bits32;
			std::bitset<64> bits64;
			switch (dataType) {
				case EtherCatData::Type::UINT8_T:	bits8 = u8; strcpy(inputString, bits8.to_string().c_str());  break;
				case EtherCatData::Type::INT8_T:	bits8 = s8; strcpy(inputString, bits8.to_string().c_str());  break;
				case EtherCatData::Type::UINT16_T:	bits16 = u16; strcpy(inputString, bits16.to_string().c_str());  break;
				case EtherCatData::Type::INT16_T:	bits16 = s16; strcpy(inputString, bits16.to_string().c_str());  break;
				case EtherCatData::Type::UINT32_T:	bits32 = u32; strcpy(inputString, bits32.to_string().c_str());  break;
				case EtherCatData::Type::INT32_T:	bits32 = s32; strcpy(inputString, bits32.to_string().c_str());  break;
				case EtherCatData::Type::UINT64_T:	bits64 = u64; strcpy(inputString, bits64.to_string().c_str());  break;
				case EtherCatData::Type::INT64_T:	bits64 = s64; strcpy(inputString, bits64.to_string().c_str());  break;
				case EtherCatData::Type::STRING:	return ImGui::InputText("##input", stringBuffer, stringBufferSize); break;
			}
			switch (dataType) {
				case EtherCatData::Type::UINT8_T:	wasEdited = ImGui::InputText("##inputBin", inputString, 9, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT8_T:	wasEdited = ImGui::InputText("##inputBin", inputString, 9, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT16_T:	wasEdited = ImGui::InputText("##inputBin", inputString, 17, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT16_T:	wasEdited = ImGui::InputText("##inputBin", inputString, 17, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT32_T:	wasEdited = ImGui::InputText("##inputBin", inputString, 33, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT32_T:	wasEdited = ImGui::InputText("##inputBin", inputString, 33, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::UINT64_T:	wasEdited = ImGui::InputText("##inputBin", inputString, 65, ImGuiInputTextFlags_AutoSelectAll); break;
				case EtherCatData::Type::INT64_T:	wasEdited = ImGui::InputText("##inputBin", inputString, 65, ImGuiInputTextFlags_AutoSelectAll); break;
			}
			if (wasEdited) {
				if (strlen(inputString) == 0) { sprintf(inputString, "0"); }
				switch (dataType) {
					case EtherCatData::Type::UINT8_T:	u8 = strtol(inputString, NULL, 2); break;
					case EtherCatData::Type::INT8_T:	s8 = strtol(inputString, NULL, 2); break;
					case EtherCatData::Type::UINT16_T:	u16 = strtol(inputString, NULL, 2);  break;
					case EtherCatData::Type::INT16_T:	s16 = strtol(inputString, NULL, 2);  break;
					case EtherCatData::Type::UINT32_T:	u32 = strtol(inputString, NULL, 2);  break;
					case EtherCatData::Type::INT32_T:	s32 = strtol(inputString, NULL, 2);  break;
					case EtherCatData::Type::UINT64_T:	u64 = strtol(inputString, NULL, 2);  break;
					case EtherCatData::Type::INT64_T:	s64 = strtol(inputString, NULL, 2);  break;
				}
				return true;
			}
			break;
	}
	return false;
}

bool EtherCatBaseData::dataTypeSelectorGui() {
	bool newTypeSelected = false;
	if (ImGui::BeginCombo("##DataTypeSelector", getTypeString())) {
		for (EtherCatData& type : dataTypes) {
			if (type.type == EtherCatData::Type::NONE) continue;
			if (ImGui::Selectable(type.displayName, type.type == dataType)) {
				dataType = type.type;
				newTypeSelected = true;
				if (type.type == EtherCatData::Type::STRING) dataFormat = DataFormat::Type::STRING;
				else if (dataFormat == DataFormat::Type::STRING) dataFormat = DataFormat::Type::DECIMAL;
			}
		}
		ImGui::EndCombo();
	}
	return newTypeSelected;
}

void EtherCatBaseData::dataFormatSelectorGui() {
	if (ImGui::BeginCombo("##DataRepresentation", getRepresentationString())) {
		for (DataFormat& format : dataRepresentations) {
			bool disableEntry = (dataType == EtherCatData::Type::STRING && format.type != DataFormat::STRING) || (dataType != EtherCatData::Type::STRING && format.type == DataFormat::STRING);
			if (disableEntry) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::Selectable(format.displayName, dataFormat == format.type)) {
				dataFormat = format.type;
			}
			if (disableEntry) END_DISABLE_IMGUI_ELEMENT
		}
		ImGui::EndCombo();
	}
}

bool EtherCatCoeData::indexEditFieldGui() {
	static char indexInput[5];
	sprintf(indexInput, "%X", index);
	if (ImGui::InputText("##Index", indexInput, 5, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll)) {
		if (strcmp(indexInput, "") == 0) index = 0;
		std::stringstream hexStream;
		hexStream << std::hex << indexInput;
		hexStream >> index;
		return true;
	}
	return false;
}

bool EtherCatCoeData::subindexEditFieldGui() {
	//for some reason this method doesn't accept uint8_t values
	//so we use a dummy uin16_t value and convert back to uint8_t after processing
	uint16_t subindex16 = subindex;
	static char subIndexInput[3];
	sprintf(subIndexInput, "%X", subindex16);
	if (ImGui::InputText("##SubIndex", subIndexInput, 3, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll)) {
		if (strcmp(subIndexInput, "") == 0) subindex16 = 0;
		std::stringstream hexStream;
		hexStream << std::hex << subIndexInput;
		hexStream >> subindex16;
		subindex = subindex16;
		return true;
	}
	return false;
}

bool EtherCatRegisterData::registerEditFieldGui() {
	static char registerInput[5];
	sprintf(registerInput, "%X", registerAddress);
	if (ImGui::InputText("##Index", registerInput, 5, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll)) {
		if (strcmp(registerInput, "") == 0) registerAddress = 0;
		std::stringstream hexStream;
		hexStream << std::hex << registerInput;
		hexStream >> registerAddress;
		return true;
	}
	return false;
}

bool EtherCatEepromData::addressFieldGui() {
	static char addressInput[5];
	sprintf(addressInput, "%X", address);
	if (ImGui::InputText("##Index", addressInput, 5, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll)) {
		if (strcmp(addressInput, "") == 0) address = 0;
		std::stringstream hexStream;
		hexStream << std::hex << addressInput;
		hexStream >> address;
		return true;
	}
	return false;
}