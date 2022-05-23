#include <pch.h>

#include "ApplicationWindow.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Fieldbus/EtherCatFieldbus.h"
#include "Gui/Fieldbus/EtherCatGui.h"
#include "Environnement/Environnement.h"

#include "Gui.h"
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include <GLFW/glfw3.h>

#include "Networking/NetworkDevice.h"

#include "Gui/Plot/Sequencer.h"

#include "Plot/Plot.h"
#include "Gui/Plot/PlotGui.h"

#include "Gui/Plot/PlaybackGui.h"

namespace Gui {



	void toolbar(float height) {

		float labelHeight = ImGui::GetTextLineHeight();
		float buttonHeight = height - ImGui::GetStyle().ItemSpacing.y - labelHeight - ImGui::GetStyle().WindowPadding.y * 2.0;
		glm::vec2 buttonSize(ImGui::GetTextLineHeight() * 4.0, buttonHeight);
		glm::vec2 windowPos = ImGui::GetWindowPos();
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.25));
		
		auto spacer = [](){
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetWindowSize().y));
			ImGui::SameLine();
		};
		
		
		//================= Environnement Control =====================
		
		ImGui::BeginGroup();
		ImGui::BeginGroup();
		
		ImGui::BeginDisabled(!Environnement::isReady() || Environnement::isStarting());
		if(Environnement::isStarting()){
			ImGui::BeginDisabled();
			ImGui::Button("Starting", buttonSize);
			ImGui::EndDisabled();
		}
		else if(!Environnement::isRunning()){
			if(ImGui::Button("Start", buttonSize)){
				if(!Environnement::isSimulating()) openEtherCatStartModal();
				Environnement::start();
			}
		}else{
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			if(ImGui::Button("Stop", buttonSize)) Environnement::stop();
			ImGui::PopStyleColor();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
			
		static ToggleSwitch simulationToggle;
		bool b_simulation = Environnement::isSimulating();
		
		bool disableSimulationSwitch = Environnement::isRunning() || Environnement::isStarting();
		ImGui::BeginDisabled(disableSimulationSwitch);
		if(simulationToggle.draw("SimulationSwitch", b_simulation, "Simulation", "Hardware", buttonSize)) {
			b_simulation = !b_simulation;
			Environnement::setSimulation(b_simulation);
		}
		ImGui::EndDisabled();
		
		ImGui::EndGroup();
		
		ImGui::PushFont(Fonts::sansRegular12);
		backgroundText("Environnement", glm::vec2(ImGui::GetItemRectSize().x, labelHeight), ImColor(0.3f, 0.3f, 0.3f, 1.0f));
		ImGui::PopFont();
		ImGui::EndGroup();
		
		
		spacer();
		
		
		
		
		
		
		//================= Machine State Control =====================

		ImGui::BeginGroup();
		ImGui::BeginGroup();
		
		ImGui::BeginDisabled(!Environnement::isRunning());
		
		if (Environnement::areAllMachinesEnabled()) {
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			ImGui::BeginDisabled();
			ImGui::Button("Enable All", buttonSize);
			ImGui::EndDisabled();
		}
		else {
			if (Environnement::areNoMachinesEnabled()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
			else ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
			if (ImGui::Button("Enable All", buttonSize)) Environnement::enableAllMachines();
		}
		ImGui::PopStyleColor();
		
		ImGui::SameLine();
		
		if(Environnement::areNoMachinesEnabled()){
			ImGui::BeginDisabled();
			ImGui::Button("Disable All", buttonSize);
			ImGui::EndDisabled();
		}else{
			if(ImGui::Button("Disable All", buttonSize)) Environnement::disableAllMachines();
		}
		
		ImGui::EndDisabled();

		ImGui::EndGroup();
		
		ImGui::PushFont(Fonts::sansRegular12);
		backgroundText("Machines", glm::vec2(ImGui::GetItemRectSize().x, labelHeight), ImColor(0.3f, 0.3f, 0.3f, 1.0f));
		ImGui::PopFont();
		
		ImGui::EndGroup();
		
		spacer();
		
		
		
		
		//================= Sequencer Transport Control =====================
		
		ImGui::BeginGroup();
		ImGui::BeginGroup();
		
		Sequencer::Gui::transportControls(buttonSize.y);
		
		ImGui::EndGroup();
		
		ImGui::PushFont(Fonts::sansRegular12);
		backgroundText("Sequencer", glm::vec2(ImGui::GetItemRectSize().x, labelHeight), ImColor(0.3f, 0.3f, 0.3f, 1.0f));
		ImGui::PopFont();
		
		ImGui::EndGroup();
		
		spacer();
		
		
		//================= Manoeuvre Playback Control =====================
		
		ImGui::BeginGroup();
		
		ImGui::BeginGroup();
		Playback::Gui::manoeuvrePlaybackControls(buttonSize.y);
		ImGui::EndGroup();
		
		ImGui::PushFont(Fonts::sansRegular12);
		backgroundText("Manoeuvre Playback", glm::vec2(ImGui::GetItemRectSize().x, labelHeight), ImColor(0.3f, 0.3f, 0.3f, 1.0f));
		ImGui::PopFont();
		
		ImGui::EndGroup();
		
		spacer();
		
		//========== Status Displays ==========
		
		ImGui::BeginGroup();
		ImGui::BeginGroup();
		
		glm::vec4 etherCatStatusColor;
		if(EtherCatFieldbus::isRunning()) etherCatStatusColor = Colors::green;
		else if(EtherCatFieldbus::isStarting()) etherCatStatusColor = Colors::yellow;
		else if(!EtherCatFieldbus::hasNetworkInterface()) etherCatStatusColor = Colors::red;
		else if(EtherCatFieldbus::slaves.empty()) etherCatStatusColor = Colors::orange;
		else etherCatStatusColor = Colors::blue;
		
		backgroundText("EtherCAT", buttonSize, etherCatStatusColor);
		
		for(auto& networkDevice : Environnement::getNetworkDevices()){
			ImGui::SameLine();
			glm::vec4 networkDeviceStatusColor;
			if(networkDevice->isConnected()) networkDeviceStatusColor = Colors::green;
			else if(networkDevice->isDetected()) networkDeviceStatusColor = Colors::blue;
			else networkDeviceStatusColor = Colors::red;
			backgroundText(networkDevice->getName(), buttonSize, networkDeviceStatusColor);
		}
		
		ImGui::EndGroup();
		
		ImGui::PushFont(Fonts::sansRegular12);
		backgroundText("Status", glm::vec2(ImGui::GetItemRectSize().x, labelHeight), ImColor(0.3f, 0.3f, 0.3f, 1.0f));
		ImGui::PopFont();
		
		ImGui::EndGroup();
		
		
		ImGui::PopStyleVar();
	}

}
