#include <pch.h>

#include "Gui/Gui.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatSlave.h"

void EtherCatSlave::nodeSpecificGui() {

    deviceSpecificGui();

    if (ImGui::BeginTabItem("EtherCAT")) {
        if (ImGui::BeginChild("EtherCatConfig")) {
        
            generalGui();
            ImGui::Separator();

            if (ImGui::BeginTabBar("EtherCatConfigTabBar")) {

                if (isSlaveKnown() && ImGui::BeginTabItem("PDO Data")) {
                    pdoDataGui();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Data Exchange")) {
                    sendReceiveEtherCatRegisterGui();
                    ImGui::Separator();
                    ImGui::Spacing();
                    sendReceiveCanOpenGui();
                    ImGui::Separator();
                    ImGui::Spacing();
                    sendReceiveEepromGui();

                    ImGui::EndTabItem();
                }

                if (isDetected() && ImGui::BeginTabItem("Generic Info")) {
                    genericInfoGui();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }



            ImGui::EndChild();
        }
        ImGui::EndTabItem();
    }
}
    
void EtherCatSlave::generalGui() {

    static glm::vec4 blueColor = glm::vec4(0.1, 0.1, 0.4, 1.0);
    static glm::vec4 redColor = glm::vec4(0.7, 0.1, 0.1, 1.0);
    static glm::vec4 greenColor = glm::vec4(0.3, 0.7, 0.1, 1.0);
    static glm::vec4 yellowColor = glm::vec4(0.8, 0.8, 0.0, 1.0);
    static glm::vec4 grayColor = glm::vec4(0.5, 0.5, 0.5, 1.0);

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("Device Status");
    ImGui::PopFont();

    float displayWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
    glm::vec2 statusDisplaySize(displayWidth, ImGui::GetTextLineHeight() * 2.0);
    static float maxStatusDisplayWidth = ImGui::GetTextLineHeight() * 10.0;
    if (statusDisplaySize.x > maxStatusDisplayWidth) statusDisplaySize.x = maxStatusDisplayWidth;

    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::PushStyleColor(ImGuiCol_Button, isOnline() ? greenColor : (isDetected() ? yellowColor : redColor));
    ImGui::Button(isOnline() ? "Online" : (isDetected() ? "Detected" : "Offline"), statusDisplaySize);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    if (!isOnline()){
        ImGui::PushStyleColor(ImGuiCol_Button, blueColor);
        ImGui::PushStyleColor(ImGuiCol_Text, grayColor);
        ImGui::Button("No State", statusDisplaySize);
        ImGui::PopStyleColor(2);
    }else{
        if (isStateBootstrap() || isStateInit()) ImGui::PushStyleColor(ImGuiCol_Button, redColor);
        else if (isStatePreOperational() || isStateSafeOperational()) ImGui::PushStyleColor(ImGuiCol_Button, yellowColor);
        else ImGui::PushStyleColor(ImGuiCol_Button, greenColor);
        ImGui::Button(getStateChar(), statusDisplaySize);
        ImGui::PopStyleColor();
    }
    ImGui::PopFont();
    ImGui::PopItemFlag();

    ImGui::PushFont(Fonts::robotoBold15);
    ImGui::Text("Station Alias");
    ImGui::PopFont();
    ImGui::InputScalar("##stationAlias", ImGuiDataType_U16, &stationAlias);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("This is the manual address of the EtherCAT device."
                                                  "\nThe station alias needs to match the device for it to be recognized."
                                                  "\nAddresses range from 0 to 65535.");
}

void EtherCatSlave::genericInfoGui() {

    ImGui::Text("Manual Address: %i", getStationAlias());
    ImGui::Text("Assigned Address: %i", getAssignedAddress());

    ImGui::Text("Device Name: %s", identity->name);

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
        ImGui::Text("SDO info: %s", supportsCoE_SDOinfo() ? "supported" : "not supported");
        ImGui::Text("PDO assign: %s", supportsCoE_PDOassign() ? "supported" : "not supported");
        ImGui::Text("PDO config: %s", supportsCoE_PDOconfig() ? "supported" : "not supported");
        ImGui::Text("Upload: % s", supportsCoE_upload() ? "supported" : "not supported");
        ImGui::Text("SDO Complete Access: %s", supportsCoE_SDOCA() ? "supported" : "not supported");
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

void EtherCatSlave::pdoDataGui() {

    static auto displayPDO = [](EtherCatPdoAssignement& pdo, const char* pdoName) {
        ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
        if (ImGui::BeginTable(pdoName, 5, tableFlags)) {
            ImGui::TableSetupColumn("Index");
            ImGui::TableSetupColumn("Subindex");
            ImGui::TableSetupColumn("Bytes");
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

    ImGui::Text("RX-PDO (outputs received by slave)");
    displayPDO(rxPdoAssignement, "RX-PDO");
    ImGui::Text("TX-PDO (inputs sent by slave)");
    displayPDO(txPdoAssignement, "TX-PDO");
}








void EtherCatSlave::sendReceiveCanOpenGui() {

    ImGui::PushID("Coe");

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("CanOpen data");
    ImGui::PopFont();

    bool allowCoeSendReceive = false;
    if (isDetected() && isCoeSupported()) allowCoeSendReceive = true;

    if (!isDetected()) ImGui::TextWrapped("Sending and Receving CanOpen Data is disabled while the device is not detected");
    else if (!isCoeSupported()) ImGui::TextWrapped("Sending and Received CanOpen Data is disabled because the device doesn't support CanOpen over EtherCAT");

    if (!allowCoeSendReceive) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(0.5, 0.5, 0.5, 1.0));
    }

    ImGui::PushID("DataUpload");
    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
    if (ImGui::BeginTable("##ConfigurationData", 4, tableFlags)) {
        ImGui::TableSetupColumn("Index");
        ImGui::TableSetupColumn("Sub");
        ImGui::TableSetupColumn("Datatype");
        ImGui::TableSetupColumn("Format");
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::SetNextItemWidth(ImGui::CalcTextSize("Index").x);
        if (uploadCoeData.indexEditFieldGui()) uploadCoeData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(ImGui::CalcTextSize("Sub").x);
        if (uploadCoeData.subindexEditFieldGui()) uploadCoeData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(2);
        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
        if (uploadCoeData.dataTypeSelectorGui()) uploadCoeData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(3);
        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
        uploadCoeData.dataFormatSelectorGui();
        ImGui::EndTable();
    }
    
    ImGui::SetNextItemWidth(ImGui::GetItemRectSize().x);
    uploadCoeData.dataEditFieldGui();

    ImGui::PopID();

    if(ImGui::Button("Upload")) {
        std::thread dataUploader = std::thread([this]() {
            uploadCoeData.b_hasTransferred = true;
            uploadCoeData.b_isTransfering = true;
            uploadCoeData.b_transferSuccessfull = uploadCoeData.write(getSlaveIndex());
            uploadCoeData.b_isTransfering = false;
        });
        dataUploader.detach();
    }
    if (uploadCoeData.b_hasTransferred) {
        ImGui::SameLine();
        ImGui::Text(uploadCoeData.b_isTransfering ? "Uploading..." : (uploadCoeData.b_transferSuccessfull ? "Upload Successfull" : "Upload Failed"));
    }

    
    ImGui::Separator();

    ImGui::PushID("DownloadData");
    if (ImGui::BeginTable("##ConfigurationData", 4, tableFlags)) {
        ImGui::TableSetupColumn("Index");
        ImGui::TableSetupColumn("Sub");
        ImGui::TableSetupColumn("Datatype");
        ImGui::TableSetupColumn("Format");
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::SetNextItemWidth(ImGui::CalcTextSize("Index").x);
        if (downloadCoeData.indexEditFieldGui()) downloadCoeData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(ImGui::CalcTextSize("Sub").x);
        if (downloadCoeData.subindexEditFieldGui()) downloadCoeData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(2);
        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
        if (downloadCoeData.dataTypeSelectorGui()) downloadCoeData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(3);
        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
        downloadCoeData.dataFormatSelectorGui();
        ImGui::EndTable();
    }

    ImGui::SetNextItemWidth(ImGui::GetItemRectSize().x);
    downloadCoeData.valueTextGui();

    ImGui::PopID();

    static bool uploadResult;
    if (ImGui::Button("Download")) {
        std::thread dataDownloader = std::thread([this]() {
            downloadCoeData.b_hasTransferred = true;
            downloadCoeData.b_isTransfering = true;
            downloadCoeData.b_transferSuccessfull = downloadCoeData.read(getSlaveIndex());
            downloadCoeData.b_isTransfering = false;
        });
        dataDownloader.detach();
    }

    if (downloadCoeData.b_hasTransferred) {
        ImGui::SameLine();
        ImGui::Text(downloadCoeData.b_isTransfering ? "Downloading..." : (downloadCoeData.b_transferSuccessfull ? "Download Successfull" : "Download Failed"));
    }

    if (!allowCoeSendReceive) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }

    ImGui::PopID();
    
}





void EtherCatSlave::sendReceiveEtherCatRegisterGui() {

    ImGui::PushID("ESC");

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("EtherCAT register data");
    ImGui::PopFont();

    bool allowRegisterSendReceive = isDetected();
    if (!isDetected()) ImGui::TextWrapped("Sending and Receving CanOpen Data is disabled while the device is not detected");

    if (!allowRegisterSendReceive) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(0.5, 0.5, 0.5, 1.0));
    }

    ImGui::PushID("DataUpload");
    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
    if (ImGui::BeginTable("##ConfigurationData", 3, tableFlags)) {
        ImGui::TableSetupColumn("Register");
        ImGui::TableSetupColumn("Datatype");
        ImGui::TableSetupColumn("Format");
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::SetNextItemWidth(ImGui::CalcTextSize("Register").x);
        if (uploadRegisterData.registerEditFieldGui()) uploadRegisterData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
        if (uploadRegisterData.dataTypeSelectorGui()) uploadRegisterData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(2);
        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
        uploadRegisterData.dataFormatSelectorGui();
        ImGui::EndTable();
    }

    ImGui::SetNextItemWidth(ImGui::GetItemRectSize().x);
    uploadRegisterData.dataEditFieldGui();
    
    ImGui::PopID();

    if (ImGui::Button("Upload")) {
        std::thread dataUploader = std::thread([this]() {
            uploadRegisterData.b_hasTransferred = true;
            uploadRegisterData.b_isTransfering = true;
            uploadRegisterData.b_transferSuccessfull = uploadRegisterData.write(getAssignedAddress());
            uploadRegisterData.b_isTransfering = false;
        });
        dataUploader.detach();
    }
    if (uploadRegisterData.b_hasTransferred) {
        ImGui::SameLine();
        ImGui::Text(uploadRegisterData.b_isTransfering ? "Uploading..." : (uploadRegisterData.b_transferSuccessfull ? "Upload Successfull" : "Upload Failed"));
    }


    ImGui::Separator();

    ImGui::PushID("DownloadData");
    if (ImGui::BeginTable("##ConfigurationData", 3, tableFlags)) {
        ImGui::TableSetupColumn("Register");
        ImGui::TableSetupColumn("Datatype");
        ImGui::TableSetupColumn("Format");
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::SetNextItemWidth(ImGui::CalcTextSize("Register").x);
        if (downloadRegisterData.registerEditFieldGui()) downloadRegisterData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 6.0);
        if (downloadRegisterData.dataTypeSelectorGui()) downloadRegisterData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(2);
        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
        downloadRegisterData.dataFormatSelectorGui();
        ImGui::EndTable();
    }

    ImGui::SetNextItemWidth(ImGui::GetItemRectSize().x);
    downloadRegisterData.valueTextGui();

    ImGui::PopID();

    static bool uploadResult;
    if (ImGui::Button("Download")) {
        std::thread dataDownloader = std::thread([&]() {
            downloadRegisterData.b_hasTransferred = true;
            downloadRegisterData.b_isTransfering = true;
            downloadRegisterData.b_transferSuccessfull = downloadRegisterData.read(getAssignedAddress());
            downloadRegisterData.b_isTransfering = false;
        });
        dataDownloader.detach();
    }

    if (downloadRegisterData.b_hasTransferred) {
        ImGui::SameLine();
        ImGui::Text(downloadRegisterData.b_isTransfering ? "Downloading..." : (downloadRegisterData.b_transferSuccessfull ? "Download Successfull" : "Download Failed"));
    }

    if (!allowRegisterSendReceive) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }

    ImGui::PopID();
    
}




void EtherCatSlave::sendReceiveEepromGui() {

    ImGui::PushID("EPPROM");

    ImGui::PushFont(Fonts::robotoBold20);
    ImGui::Text("EEPROM data");
    ImGui::PopFont();

    bool allowRegisterSendReceive = isDetected();
    if (!isDetected()) ImGui::TextWrapped("Sending and Receving CanOpen Data is disabled while the device is not detected");

    if (!allowRegisterSendReceive) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(0.5, 0.5, 0.5, 1.0));
    }

    ImGui::PushID("DataUpload");
    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
    if (ImGui::BeginTable("##ConfigurationData", 2, tableFlags)) {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Format");
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::SetNextItemWidth(ImGui::CalcTextSize("Address").x);
        if (uploadEepromData.addressFieldGui()) uploadEepromData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
        uploadEepromData.dataFormatSelectorGui();
        ImGui::EndTable();
    }

    ImGui::SetNextItemWidth(ImGui::GetItemRectSize().x);
    uploadEepromData.dataEditFieldGui();

    ImGui::PopID();

    if (ImGui::Button("Upload")) {
        std::thread dataUploader = std::thread([this]() {
            uploadEepromData.b_hasTransferred = true;
            uploadEepromData.b_isTransfering = true;
            uploadEepromData.b_transferSuccessfull = uploadEepromData.write(getSlaveIndex());
            uploadEepromData.b_isTransfering = false;
            });
        dataUploader.detach();
    }
    if (uploadEepromData.b_hasTransferred) {
        ImGui::SameLine();
        ImGui::Text(uploadEepromData.b_isTransfering ? "Uploading..." : (uploadEepromData.b_transferSuccessfull ? "Upload Successfull" : "Upload Failed"));
    }


    ImGui::Separator();

    ImGui::PushID("DownloadData");
    if (ImGui::BeginTable("##ConfigurationData", 2, tableFlags)) {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Format");
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::SetNextItemWidth(ImGui::CalcTextSize("Address").x);
        if (downloadEepromData.addressFieldGui()) downloadEepromData.b_hasTransferred = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 4.0);
        downloadEepromData.dataFormatSelectorGui();
        ImGui::EndTable();
    }

    ImGui::SetNextItemWidth(ImGui::GetItemRectSize().x);
    downloadEepromData.valueTextGui();

    ImGui::PopID();

    static bool uploadResult;
    if (ImGui::Button("Download")) {
        std::thread dataDownloader = std::thread([&]() {
            downloadEepromData.b_hasTransferred = true;
            downloadEepromData.b_isTransfering = true;
            downloadEepromData.b_transferSuccessfull = downloadEepromData.read(getSlaveIndex());
            downloadEepromData.b_isTransfering = false;
        });
        dataDownloader.detach();
    }

    if (downloadEepromData.b_hasTransferred) {
        ImGui::SameLine();
        ImGui::Text(downloadEepromData.b_isTransfering ? "Downloading..." : (downloadEepromData.b_transferSuccessfull ? "Download Successfull" : "Download Failed or Result was 0"));
    }

    if (!allowRegisterSendReceive) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor();
    }

    ImGui::PopID();

}
