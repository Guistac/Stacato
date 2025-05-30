#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Fieldbus/EtherCatFieldbus.h"
#include "Gui/Fieldbus/EtherCatGui.h"
#include "Environnement/Environnement.h"

#include "Gui.h"
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Images.h"

#include "Gui/Utilities/CustomWidgets.h"

#include <GLFW/glfw3.h>

#include "Networking/NetworkDevice.h"

#include "Gui/Plot/SequencerGui.h"

#include "Plot/Plot.h"
#include "Gui/Plot/PlotGui.h"

#include "Gui/Plot/PlaybackGui.h"

#include "Console/ConsoleHandler.h"

#include "Utilities/ProgressIndicator.h"

namespace Stacato::Gui {

	bool b_init = false;

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
		
		if(Environnement::isStarting()){
			ImGui::BeginDisabled();
			ImGui::Button("Starting", buttonSize);
			ImDrawList* canvas = ImGui::GetWindowDrawList();
			ImVec2 min = ImGui::GetItemRectMin();
			ImVec2 max = ImGui::GetItemRectMax();
			ImVec2 size = ImGui::GetItemRectSize();
			float progress = EtherCatFieldbus::startupProgress.progress;
			canvas->AddRectFilled(min, ImVec2(min.x + size.x * progress, max.y), ImColor(Colors::transparentWhite));
			ImGui::EndDisabled();
		}
		else if(Environnement::isRunning()){
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			if(ImGui::Button("Stop", buttonSize)) Environnement::stop();
			ImGui::PopStyleColor();
		}else{
			ImGui::BeginDisabled(!Environnement::isReady());
			if(ImGui::Button("Start", buttonSize)) Environnement::start();
			ImGui::EndDisabled();
		}
		
		
		
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
		backgroundText("System", ImVec2(ImGui::GetItemRectSize().x, labelHeight), ImColor(0.3f, 0.3f, 0.3f, 1.0f));
		ImGui::PopFont();
		ImGui::EndGroup();
		
		
		spacer();
		
		
		//========== Status Displays ==========
		
		ImGui::BeginGroup();
		ImGui::BeginGroup();
		
		glm::vec4 etherCatStatusColor;
		if(EtherCatFieldbus::isRunning()) etherCatStatusColor = Colors::green;
		else if(EtherCatFieldbus::isStarting()) etherCatStatusColor = Colors::yellow;
		else if(EtherCatFieldbus::isScanning()) etherCatStatusColor = Colors::darkYellow;
		else if(EtherCatFieldbus::isInitialized()) etherCatStatusColor = Colors::orange;
		else etherCatStatusColor = Colors::blue;
		
		if(ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt)){
			ImGui::BeginDisabled(!EtherCatFieldbus::canScan());
			if (ImGui::Button("Scan", buttonSize)) EtherCatFieldbus::scan();
			ImGui::EndDisabled();
		}else{
			backgroundText("EtherCAT", buttonSize, etherCatStatusColor);
			if(ImGui::IsItemHovered()){
				ImGui::BeginTooltip();
				if(EtherCatFieldbus::isRunning()) ImGui::Text("EtherCAT Fieldbus is running.\nAll devices are operational");
				else if(EtherCatFieldbus::isStarting()) ImGui::Text("EtherCAT Fieldbus is starting.");
				else if(EtherCatFieldbus::isScanning()) ImGui::Text("EtherCAT Fieldbus is scanning for devices.");
				else if(EtherCatFieldbus::isInitialized()) ImGui::Text("EtherCAT Fieldbus is initialized and has discovered devices.");
				else ImGui::Text("EtherCAT Fieldbus is not initialized, no devices are detected.");
				ImGui::EndTooltip();
			}
		}
		
		
		for(int i = 0; i < Environnement::getNetworkDevices().size(); i++){
			auto networkDevice = Environnement::getNetworkDevices()[i];
			ImGui::PushID(i);
			
			ImGui::SameLine();
			glm::vec4 networkDeviceStatusColor;
			if(networkDevice->isConnected()) networkDeviceStatusColor = Colors::green;
			else if(networkDevice->isDetected()) networkDeviceStatusColor = Colors::blue;
			else networkDeviceStatusColor = Colors::red;
			
			ImVec2 cursor = ImGui::GetCursorPos();
			ImGui::BeginDisabled();
			ImGui::Dummy(buttonSize);
			ImGui::EndDisabled();
			ImGui::SetCursorPos(cursor);
			ImRect hoverRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
			if(hoverRect.Contains(ImGui::GetMousePos())){
				ImGui::SetCursorPos(cursor);
				if(networkDevice->isConnected()){
					if(customButton("Stop", buttonSize, Colors::green, ImGui::GetStyle().FrameRounding, ImDrawFlags_None)) {
						networkDevice->disconnect();
					}
				}
				else{
					if(customButton("Start", buttonSize, Colors::blue, ImGui::GetStyle().FrameRounding, ImDrawFlags_None)) {
						networkDevice->connect();
					}
				}
			}
			else{
				backgroundText(networkDevice->getName(), buttonSize, networkDeviceStatusColor);
			}
			
			ImGui::PopID();
		}
		
		ImGui::EndGroup();
		
		ImGui::PushFont(Fonts::sansRegular12);
		backgroundText("Network", glm::vec2(ImGui::GetItemRectSize().x, labelHeight), ImColor(0.3f, 0.3f, 0.3f, 1.0f));
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
		
		
		
		/*
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
		 */
		
		
		//================= Manoeuvre Playback Control =====================
		
		ImGui::BeginGroup();
		
		ImGui::BeginGroup();
		Playback::Gui::manoeuvrePlaybackControls(buttonSize.y);
		ImGui::EndGroup();
		
		ImGui::PushFont(Fonts::sansRegular12);
		backgroundText("Manoeuvre Playback", glm::vec2(ImGui::GetItemRectSize().x, labelHeight), ImColor(0.3f, 0.3f, 0.3f, 1.0f));
		ImGui::PopFont();
		
		ImGui::EndGroup();
	
		
		ImGui::SameLine();
		
		
		ImGui::BeginGroup();
		ImGui::BeginGroup();
		ConsoleHandler::gui(buttonSize.y);
		ImGui::EndGroup();
        
		ImGui::PushFont(Fonts::sansRegular12);
		backgroundText("Console", glm::vec2(ImGui::GetItemRectSize().x, labelHeight), ImColor(0.3f, 0.3f, 0.3f, 1.0f));
		ImGui::PopFont();
         
		ImGui::EndGroup();
		
        
        ImGui::SameLine();
        
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		std::ostringstream oss;
		oss << std::put_time(&tm, "%H:%M:%S");
		auto timeString = oss.str();
        
		ImGui::BeginGroup();
        ImGui::PushFont(Fonts::sansBold26);
        glm::vec2 timeSize(ImGui::CalcTextSize("00:00:00").x + ImGui::GetStyle().FramePadding.x * 2.0, buttonHeight);
		backgroundText(timeString.c_str(), timeSize, Colors::darkGray);
        ImGui::PopFont();
		
		ImGui::PushFont(Fonts::sansRegular12);
		backgroundText("Current Time", glm::vec2(timeSize.x, labelHeight), ImColor(0.3f, 0.3f, 0.3f, 1.0f));
		ImGui::PopFont();
		ImGui::EndGroup();
		
		
		ImGui::PopStyleVar();
	}

}
