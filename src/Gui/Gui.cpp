#include "Gui.h"

#include "EtherCatGui.h"

#include <imgui.h>

#include "Fieldbus/EtherCatFieldbus.h"

bool fieldbusSelection = true;
bool fieldbusStarted = false;
bool startup = true;

void gui() {

	if (startup) {
		startup = false;
		EtherCatFieldbus::updateNetworkInterfaceCardList();
		EtherCatFieldbus::init(EtherCatFieldbus::networkInterfaceCards.front());
		fieldbusStarted = true;
	}

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);

	if (!fieldbusStarted) {

		ImGui::Begin("Fieldbus Test Program");
		if (fieldbusSelection) {
			ImGui::OpenPopup("Fieldbus Selection");
			fieldbusSelection = false;
		}

		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Fieldbus Selection", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			static bool start = true;
			if (start) {
				start = false;
				EtherCatFieldbus::updateNetworkInterfaceCardList();
			}
			ImGui::Text("EtherCAT Fieldbus Parameters");
			ImGui::Separator();
			ImGui::Text("Network Interface Card Selection");
				
			std::vector<NetworkInterfaceCard>& nics = EtherCatFieldbus::networkInterfaceCards;
			static int selectedNic = -1;
			const char* label = (selectedNic == -1 || selectedNic >= nics.size()) ? "Select NIC" : nics[selectedNic].description;
			if (ImGui::BeginCombo("##Network Interface Card", label)) {
				for (int i = 0; i < nics.size(); i++) {
					bool selected = i == selectedNic;
					if (ImGui::Selectable(nics[i].description, selected)) {
						selectedNic = i;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			if (ImGui::Button("Refresh List")) EtherCatFieldbus::updateNetworkInterfaceCardList();
			ImGui::Text("Nic ID: %s", (selectedNic != -1 && selectedNic < nics.size()) ? nics[selectedNic].name : "No Selection");

			if (ImGui::Button("Start EtherCAT Fieldbus")) {
				if (selectedNic != -1 && selectedNic < nics.size()) {
                    if(EtherCatFieldbus::init(nics[selectedNic])){
                        fieldbusStarted = true;
                        ImGui::CloseCurrentPopup();
                    }
				}
			}
			ImGui::Separator();
		}
		ImGui::EndPopup();
		

		ImGui::End();
	}
	else {
		if (!etherCatGui()) {
			EtherCatFieldbus::terminate();
			fieldbusStarted = false;
			fieldbusSelection = true;
		}
	}
}


void quit() {
	if(fieldbusStarted) EtherCatFieldbus::terminate();
}
