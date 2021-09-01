#include <pch.h>

#include "Gui.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "GuiWindow.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatSlave.h"

void etherCatSlaves() {

	ImGui::BeginGroup();

	if (EtherCatFieldbus::b_processRunning || EtherCatFieldbus::b_processStarting) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0, 0.0, 0.0, 1.0));
	}
	if (ImGui::Button("Scan Network")) EtherCatFieldbus::scanNetwork();
	if (EtherCatFieldbus::b_processRunning || EtherCatFieldbus::b_processStarting) {
		ImGui::PopItemFlag();
		ImGui::PopStyleColor();
	}
	ImGui::SameLine();
	ImGui::Text("%i Devices Found", EtherCatFieldbus::slaves.size());

	static int selectedSlaveIndex = -1;

	ImVec2 listWidth(ImGui::GetTextLineHeight() * 14, ImGui::GetContentRegionAvail().y);
	if (ImGui::BeginListBox("##DiscoveredEtherCATSlaves", listWidth)) {
		for (auto slave : EtherCatFieldbus::slaves) {
			bool selected = selectedSlaveIndex == slave->getSlaveIndex();
			if (ImGui::Selectable(slave->getName(), &selected)) selectedSlaveIndex = slave->getSlaveIndex();
		}
		if (EtherCatFieldbus::slaves.empty()) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::Selectable("No Devices Detected...");
			ImGui::PopItemFlag();
		}
		ImGui::EndListBox();
	}
	ImGui::EndGroup();
	ImGui::SameLine();

    std::shared_ptr<EtherCatSlave> selectedSlave = nullptr;
	for (auto slave : EtherCatFieldbus::slaves) 
		if (slave->getSlaveIndex() == selectedSlaveIndex) { selectedSlave = slave; break; }

	ImGui::BeginGroup();
	ImGui::PushFont(GuiWindow::robotoBold20);
	if (selectedSlave) ImGui::Text("%s (Node #%i, Address: %i) ", selectedSlave->getDeviceName(), selectedSlave->getSlaveIndex(), selectedSlave->getStationAlias());
	else ImGui::Text("No Device Selected");
	ImGui::PopFont();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::BeginChild(ImGui::GetID("SelectedSlaveDisplayWindow"))) {
		if (selectedSlave) {
			selectedSlave->gui();
		}
		ImGui::EndChild();
	}
	ImGui::PopStyleVar();
	ImGui::EndGroup();

}

void EtherCatSlave::gui() {
    if (ImGui::BeginTabBar("DeviceTabBar")) {
        deviceSpecificGui();

        if (isDeviceKnown() && ImGui::BeginTabItem("IO Data")) {
            ioDataGui();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Generic Info")) {
            genericInfoGui();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void EtherCatSlave::genericInfoGui() {

    ImGui::Text("Manual Address: %i", getStationAlias());
    ImGui::Text("Assigned Address: %i", getAssignedAddress());

    ImGui::Separator();

    ImGui::Text("state: %s (%s)", getStateChar(), hasStateError() ? "State Error" : "No Error");
    ImGui::Text("ALstatuscode: %i : %s", identity->ALstatuscode, ec_ALstatuscode2string(identity->ALstatuscode));

    ImGui::Separator();

    ImGui::Text("Physical Type: %i", identity->ptype);
    ImGui::Text("Topology: %i", identity->topology);
    ImGui::Text("Active Ports: %i", identity->activeports);
    ImGui::Text("Consumed Ports: %i", identity->consumedports);

    ImGui::Text("Parent: %i", identity->parent);
    ImGui::Text("Parent Port: %i", identity->parentport);
    ImGui::Text("Entry Port: %i", identity->entryport);
    ImGui::Text("Ebus Current: %i", identity->Ebuscurrent);

    ImGui::Separator();

    if (!b_mapped) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4, 0.4, 0.4, 1.0));
    }

    ImGui::Text("Input Bytes: %i (%i bits)", identity->Ibytes, identity->Ibits);
    ImGui::Text("Output Bytes: %i (%i bits)", identity->Obytes, identity->Obits);

    ImGui::Separator();

    ImGui::Text("hasDC: %i", identity->hasdc);
    ImGui::Text("DCactive: %i", identity->DCactive);

    ImGui::Text("pdelay: %i", identity->pdelay);
    ImGui::Text("DC next: %i", identity->DCnext);
    ImGui::Text("DC previous: %i", identity->DCprevious);
    ImGui::Text("DC cycle: %i", identity->DCcycle);
    ImGui::Text("DC shift: %i", identity->DCshift);

    ImGui::Text("DC rtA: %i", identity->DCrtA);
    ImGui::Text("DC rtB: %i", identity->DCrtB);
    ImGui::Text("DC rtC: %i", identity->DCrtC);
    ImGui::Text("DC rtD: %i", identity->DCrtD);

    ImGui::Separator();

    ImGui::Text("EEPROM Manufacturer: %i", identity->eep_man);
    ImGui::Text("EEPROM ID: %i", identity->eep_id);
    ImGui::Text("EEPROM Revision: %i", identity->eep_rev);

    ImGui::Separator();

    ImGui::Text("Interface Type: %i", identity->Itype);
    ImGui::Text("Device Type: %i", identity->Dtype);

    ImGui::Separator();

    ImGui::Text("Sync Manager Type:");
    static const char* smTypeChars[5] = {
        "Unused",
        "MailboxRead",
        "MailboxWrite",
        "Outputs",
        "Inputs"
    };
    ImGui::Text("SM0: %s", smTypeChars[identity->SMtype[0]]);
    ImGui::Text("SM1: %s", smTypeChars[identity->SMtype[1]]);
    ImGui::Text("SM2: %s", smTypeChars[identity->SMtype[2]]);
    ImGui::Text("SM3: %s", smTypeChars[identity->SMtype[3]]);

    ImGui::Separator();

    ImGui::Text("Fieldbus Memory Management Units (FMMU)");
    ImGui::Text("FMMU0 function: %i", identity->FMMU0func);
    ImGui::Text("FMMU1 function: %i", identity->FMMU1func);
    ImGui::Text("FMMU2 function: %i", identity->FMMU2func);
    ImGui::Text("FMMU3 function: %i", identity->FMMU3func);
    ImGui::Text("First Unused FMMU: %i", identity->FMMUunused);

    ImGui::Separator();

    ImGui::Text("Mailbox Length: %i", identity->mbx_l);
    ImGui::Text("Mailbox Write Offset: %i", identity->mbx_wo);
    ImGui::Text("Mailbox Read Length: %i bytes", identity->mbx_rl);
    ImGui::Text("Mailbox Read Offset: %i", identity->mbx_ro);
    ImGui::Text("Mailbox Supported Protocols: %i", identity->mbx_proto);
    ImGui::Text("Mailbox Counter Value: %i", identity->mbx_cnt);

    ImGui::Separator();

    ImGui::Text("Link to Config Table: %i", identity->configindex);
    ImGui::Text("Link to SII config: %i", identity->SIIindex);
    ImGui::Text("EEPROM byte read count: %i", identity->eep_8byte);
    ImGui::Text("EEPROM master or PDI: %i", identity->eep_pdi);

    ImGui::Separator();

    if (isCoeSupported()) {
        ImGui::Text("CoE is Supported");
        ImGui::Text("SDO info: %s",             supportsCoE_SDOinfo() ? "supported" : "not supported");
        ImGui::Text("PDO assign: %s",           supportsCoE_PDOassign() ? "supported" : "not supported");
        ImGui::Text("PDO config: %s",           supportsCoE_PDOconfig() ? "supported" : "not supported");
        ImGui::Text("Upload: % s",              supportsCoE_upload() ? "supported" : "not supported");
        ImGui::Text("SDO Complete Access: %s",  supportsCoE_SDOCA() ? "supported" : "not supported");
    }
    else ImGui::Text("CoE is not Supported");

    ImGui::Text("FoE Details: %i", identity->FoEdetails);
    ImGui::Text("EoE Details: %i", identity->EoEdetails);
    ImGui::Text("SoE Details: %i", identity->SoEdetails);

    ImGui::Separator();

    ImGui::Text("Block LRW: %i", identity->blockLRW);
    ImGui::Text("Group: %i", identity->group);
    ImGui::Text("Is Lost: %i", identity->islost);

    if (!b_mapped) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }
}

void EtherCatSlave::ioDataGui() {
    static auto displayDataTable = [](std::vector<ioData*>& data, const char* tableName) {
        ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable(tableName, 3, tableFlags)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();
            for (ioData* data : data) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", data->getName());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", data->getTypeName());
                ImGui::TableSetColumnIndex(2);
                switch (data->getType()) {
                    case DataType::BOOL_VALUE:  ImGui::Text("%i", data->getBool()); break;
                    case DataType::UINT8_T:     ImGui::Text("%i", data->getUnsignedByte()); break;
                    case DataType::INT8_T:      ImGui::Text("%i", data->getSignedByte()); break;
                    case DataType::UINT16_T:    ImGui::Text("%i", data->getUnsignedShort()); break;
                    case DataType::INT16_T:     ImGui::Text("%i", data->getSignedShort()); break;
                    case DataType::UINT32_T:    ImGui::Text("%i", data->getUnsignedLong()); break;
                    case DataType::INT32_T:     ImGui::Text("%i", data->getSignedLong()); break;
                    case DataType::UINT64_T:    ImGui::Text("%i", data->getUnsignedLongLong()); break;
                    case DataType::INT64_T:     ImGui::Text("%i", data->getSignedLongLong()); break;
                    case DataType::FLOAT32:     ImGui::Text("%.5f", data->getFloat()); break;
                    case DataType::FLOAT64:     ImGui::Text("%.5f", data->getDouble()); break;
                }
            }
            ImGui::EndTable();
        }
    };

    ImGui::PushFont(GuiWindow::robotoBold20);
    ImGui::Text("Public Data");
    ImGui::PopFont();
    ImGui::Text("Input Data:");
    displayDataTable(getNodeInputData(), "Input Data");
    ImGui::Separator();
    ImGui::Text("Output Data:");
    displayDataTable(getNodeOutputData(), "Output Data");
    ImGui::Separator();

    static auto displayPDO = [](EtherCatPdoAssignement& pdo, const char* pdoName) {
        ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable(pdoName, 5, tableFlags)) {
            ImGui::TableSetupColumn("Index");
            ImGui::TableSetupColumn("Subindex");
            ImGui::TableSetupColumn("Byte Count");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();
            for (EtherCatPdoMappingModule& module : pdo.modules) {
                ImGui::TableNextRow();
                ImU32 moduleHeaderBackground = ImGui::GetColorU32(ImVec4(0.2f, 0.3f, 0.7f, 1.0f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, moduleHeaderBackground);
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Module 0x%4X", module.index);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%i entries", module.getEntryCount());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("-");
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("-");
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("-");
                for (EtherCatPdoEntry& entry : module.entries) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("0x%4X", entry.index);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%X", entry.subindex);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%i", entry.byteCount);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%s", entry.name);
                    ImGui::TableSetColumnIndex(4);
                    switch (entry.byteCount) {
                        case 1: ImGui::Text("%X", *(uint8_t*)entry.dataPointer); break;
                        case 2: ImGui::Text("%X", *(uint16_t*)entry.dataPointer); break;
                        case 4: ImGui::Text("%X", *(uint32_t*)entry.dataPointer); break;
                        case 8: ImGui::Text("%X", *(uint64_t*)entry.dataPointer); break;
                        default: ImGui::Text("unknown size"); break;
                    }
                }
            }
            ImGui::EndTable();
        }
    };

    ImGui::PushFont(GuiWindow::robotoBold20);
    ImGui::Text("Process Data Objects");
    ImGui::PopFont();

    ImGui::Text("RX-PDO (outputs received by slave)");
    displayPDO(rxPdoAssignement, "RX-PDO");
    ImGui::Text("TX-PDO (inputs sent by slave)");
    displayPDO(txPdoAssignement, "TX-PDO");
}