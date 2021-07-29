#include "Gui.h"

#include "EtherCatGui.h"
#include "EthernetIpGui.h"

#include "imgui.h"

#include "Fieldbus/EtherCAT/EtherCatFieldbus.h"
#include "Fieldbus/EthernetIP/EthernetIpFieldbus.h"

enum Fieldbus {
	EthernetIP = 0,
	EtherCAT = 1,
	None = 2
};

void gui() {

	static bool fieldbusSelection = true;
	static Fieldbus fieldbus = Fieldbus::None;
	static bool fieldbusStarted = false;

	static bool startup = true;
	if (startup) {
		startup = false;
		fieldbusStarted = true;
		fieldbus = Fieldbus::EtherCAT;
		EtherCatFieldbus::updateNetworkInterfaceCardList();
		EtherCatFieldbus::init(EtherCatFieldbus::networkInterfaceCards.front());
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
			if (fieldbus == Fieldbus::None) {
				if (ImGui::Button("EtherNET/IP")) fieldbus = Fieldbus::EthernetIP;
				ImGui::SameLine();
				if (ImGui::Button("EtherCAT")) fieldbus = Fieldbus::EtherCAT;
			}
			else if (fieldbus == Fieldbus::EthernetIP) {
				ImGui::Text("EtherNET/IP Fieldbus Parameters");
				ImGui::Separator();
				ImGui::Text("Broadcast Address for device discovery:");
				int* broadcastIp = EthernetIPFieldbus::broadcastIP;
				int& port = EthernetIPFieldbus::port;
				ImGui::InputInt4("##IP", broadcastIp);
				ImGui::Text("Port:");
				ImGui::InputInt("##Port", &port);
				for (int i = 0; i < 4; i++) {
					if (broadcastIp[i] > 255) broadcastIp[i] = 255;
					else if (broadcastIp[i] < 0) broadcastIp[i] = 0;
				}
				if (port > 65635) port = 65635;
				else if (port < 0) port = 0;
				if (ImGui::Button("Start Ethernet/IP Fieldbus")) {
					EthernetIPFieldbus::init(broadcastIp, port);
					fieldbusStarted = true;
					ImGui::CloseCurrentPopup();
				}
				ImGui::Separator();
				if (ImGui::Button("Return to Fieldbus Selection")) fieldbus = Fieldbus::None;
			}
			else if (fieldbus == Fieldbus::EtherCAT) {
				static bool start = true;
				if (start) {
					start = false;
					EtherCatFieldbus::updateNetworkInterfaceCardList();
				}
				ImGui::Text("EtherCAT Fieldbus Parameters");
				ImGui::Separator();
				ImGui::Text("Network Interface Card Selection");

				char* names[5] = { "One", "Two", "Three", "Four", "Five" };
				
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
						EtherCatFieldbus::init(nics[selectedNic]);
						fieldbusStarted = true;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::Separator();
				if (ImGui::Button("Return to Fieldbus Selection")) fieldbus = Fieldbus::None;
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}
	else if (fieldbus == Fieldbus::EthernetIP) {
		if (!ethernetIpGui()) {
			EthernetIPFieldbus::terminate();
			fieldbus = Fieldbus::None;
			fieldbusStarted = false;
			fieldbusSelection = true;
		}
	}
	else if (fieldbus == Fieldbus::EtherCAT) {
		if (!etherCatGui()) {
			EtherCatFieldbus::terminate();
			fieldbus = Fieldbus::None;
			fieldbusStarted = false;
			fieldbusSelection = true;
		}
	}
}