#include <pch.h>

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Nodes/NodeFactory.h"

namespace Environnement::NodeGraph::Gui{


	void nodeAdder() {

		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetTextLineHeight() * 1.0);

		ImGui::BeginChild("##NodeList", ImGui::GetContentRegionAvail());

			ImGui::PushFont(Fonts::sansBold20);
			ImGui::Text("Node Library");
			ImGui::PopFont();
		
			auto listNodes = [](const std::vector<Node*>& nodes){
				for (auto& node : nodes) {
					const char* nodeDisplayName = node->getName();
					ImGui::Selectable(nodeDisplayName);
					const char* nodeSaveName = node->getSaveName();
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
						ImGui::SetDragDropPayload("Node", &nodeSaveName, sizeof(const char*));
						ImGui::Text("%s", nodeDisplayName);
						ImGui::EndDragDropSource();
					}
				}
			};
		
			ImGui::PushFont(Fonts::sansBold15);
			if (ImGui::CollapsingHeader("EtherCAT Devices")) {

				ImGui::PushFont(Fonts::sansRegular15);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
				ImGui::Text("By Manufacturer");
				ImGui::PopStyleColor();
				for (auto& manufacturer : NodeFactory::getEtherCatDevicesByManufacturer()) {
					if (ImGui::TreeNode(manufacturer.name)) {
						listNodes(manufacturer.nodes);
						ImGui::TreePop();
					}
				}
				ImGui::Separator();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
				ImGui::Text("By Category");
				ImGui::PopStyleColor();
				for (auto& category : NodeFactory::getEtherCatDevicesByCategory()) {
					if (ImGui::TreeNode(category.name)) {
						listNodes(category.nodes);
						ImGui::TreePop();
					}
				}
				
				
				//scan ethercat network
				ImGui::Separator();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
				ImGui::Text("Detected Devices");
				ImGui::PopStyleColor();
				static bool b_scanningNetwork = false;
				bool disableScanButton = !EtherCatFieldbus::hasNetworkInterface() || EtherCatFieldbus::isCyclicExchangeActive() || b_scanningNetwork;
				ImGui::BeginDisabled(disableScanButton);
				if (ImGui::Button("Scan Network")) {
					std::thread etherCatNetworkScanner = std::thread([]() {
						b_scanningNetwork = true;
						EtherCatFieldbus::scanNetwork();
						b_scanningNetwork = false;
					});
					etherCatNetworkScanner.detach();
				}
				ImGui::EndDisabled();
				if (b_scanningNetwork) {
					ImGui::SameLine();
					ImGui::Text("Scanning...");
				}
				
				
				for (auto device : EtherCatFieldbus::slaves_unassigned) {
					const char* deviceDisplayName = device->getName();
					ImGui::Selectable(deviceDisplayName);
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
						ImGui::SetDragDropPayload("DetectedEtherCatDevice", &device, sizeof(std::shared_ptr<EtherCatDevice>));
						ImGui::Text("%s",deviceDisplayName);
						ImGui::EndDragDropSource();
					}
				}
				ImGui::PopFont();
			}
			ImGui::PopFont();

			ImGui::PushFont(Fonts::sansBold15);
			if (ImGui::CollapsingHeader("Motion")) {
				
				if(ImGui::TreeNode("Axis")){
					ImGui::PushFont(Fonts::sansRegular15);
					listNodes(NodeFactory::getAllAxisNodes());
					ImGui::TreePop();
					ImGui::PopFont();
				}

				if (ImGui::TreeNode("Machines")) {
					for (auto& category : NodeFactory::getMachinesByCategory()) {
						if (ImGui::TreeNode(category.name)) {
							ImGui::PushFont(Fonts::sansRegular15);
							listNodes(category.nodes);
							ImGui::PopFont();
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
				
				if (ImGui::TreeNode("Safety")) {
					ImGui::PushFont(Fonts::sansRegular15);
					listNodes(NodeFactory::getAllSafetyNodes());
					ImGui::PopFont();
					ImGui::TreePop();
				}
				
				if (ImGui::TreeNode("Adapters")) {
					ImGui::PushFont(Fonts::sansRegular15);
					listNodes(NodeFactory::getAllMotionAdapterNodes());
					ImGui::PopFont();
					ImGui::TreePop();
				}
				
			}
			ImGui::PopFont();
		
			ImGui::PushFont(Fonts::sansBold15);
			if (ImGui::CollapsingHeader("Network IO")) {
				ImGui::PushFont(Fonts::sansRegular15);
				listNodes(NodeFactory::getAllNetworkNodes());
				ImGui::PopFont();
			}
			ImGui::PopFont();

			ImGui::PushFont(Fonts::sansBold15);
			if (ImGui::CollapsingHeader("Data Processors")) {
				ImGui::PushFont(Fonts::sansRegular15);
				for (auto category : NodeFactory::getProcessorNodesByCategory()) {
					if (ImGui::TreeNode(category.name)) {
						listNodes(category.nodes);
						ImGui::TreePop();
					}
				}
				ImGui::PopFont();
			}
			ImGui::PopFont();

			ImGui::EndChild();
		
		ImGui::PopStyleVar();
	}


	std::shared_ptr<Node> acceptDraggedNode() {
		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload;
			glm::vec2 mousePosition = ImGui::GetMousePos();
			payload = ImGui::AcceptDragDropPayload("Node");
			if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
				const char* nodeSaveName = *(const char**)payload->Data;
				std::shared_ptr<Node> newNode = NodeFactory::getNodeBySaveName(nodeSaveName);
				return newNode;
			}
			payload = ImGui::AcceptDragDropPayload("DetectedEtherCatDevice");
			if (payload != nullptr && payload->DataSize == sizeof(std::shared_ptr<EtherCatDevice>)) {
				std::shared_ptr<EtherCatDevice> detectedSlave = *(std::shared_ptr<EtherCatDevice>*)payload->Data;
				EtherCatFieldbus::removeFromUnassignedSlaves(detectedSlave);
				return detectedSlave;
			}
			ImGui::EndDragDropTarget();
		}
		return nullptr;
	}




	std::shared_ptr<Node> nodeAdderContextMenu() {

		std::shared_ptr<Node> output = nullptr;
		
		auto listNodes = [&](const std::vector<Node*> nodes){
			for (auto node : nodes) {
				if (ImGui::MenuItem(node->getName())) output = node->getNewInstance();
			}
		};

		ImGui::MenuItem("Node Editor Menu", nullptr, false, false);
		ImGui::Separator();
		ImGui::MenuItem("EtherCAT devices", nullptr, false, false);
		if (ImGui::BeginMenu("By Manufaturer")) {
			for (auto manufacturer : NodeFactory::getEtherCatDevicesByManufacturer()) {
				if (ImGui::BeginMenu(manufacturer.name)) {
					listNodes(manufacturer.nodes);
					ImGui::EndMenu();
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("By Category")) {
			for (auto category : NodeFactory::getEtherCatDevicesByCategory()) {
				if (ImGui::BeginMenu(category.name)) {
					listNodes(category.nodes);
					ImGui::EndMenu();
				}
			}
			ImGui::EndMenu();
		}

		std::shared_ptr<EtherCatDevice> selectedDetectedSlave = nullptr;
		if (!EtherCatFieldbus::slaves_unassigned.empty()) {
			if (ImGui::BeginMenu("Detected Slaves")) {
				for (auto detectedSlave : EtherCatFieldbus::slaves_unassigned) {
					if (ImGui::MenuItem(detectedSlave->getName())) {
						output = detectedSlave;
						selectedDetectedSlave = detectedSlave;
						EtherCatFieldbus::removeFromUnassignedSlaves(selectedDetectedSlave);
					}
				}
				ImGui::EndMenu();
			}
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Motion")) {
			if (ImGui::BeginMenu("Axis")) {
				listNodes(NodeFactory::getAllAxisNodes());
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Machines")) {
				for (auto& category : NodeFactory::getMachinesByCategory()) {
					if (ImGui::BeginMenu(category.name)) {
						listNodes(category.nodes);
						ImGui::EndMenu();
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Safety")) {
				listNodes(NodeFactory::getAllSafetyNodes());
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Network")) {
			listNodes(NodeFactory::getAllNetworkNodes());
			ImGui::EndMenu();
		}

		ImGui::Separator();

		ImGui::MenuItem("Processing Nodes", nullptr, false, false);
		for (auto category : NodeFactory::getProcessorNodesByCategory()) {
			if (ImGui::BeginMenu(category.name)) {
				listNodes(category.nodes);
				ImGui::EndMenu();
			}
		}

		return output;
	}

}
