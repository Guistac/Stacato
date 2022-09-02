#include <pch.h>
#include "ConsoleHandler.h"

#include "Console.h"
#include "ConsoleMapping.h"

#include <imgui.h>
#include "Gui/Utilities/CustomWidgets.h"
#include "Gui/Assets/Colors.h"

namespace ConsoleHandler{

	void gui(float height){
		auto console = getConnectedConsole();
		
		float width = ImGui::GetTextLineHeight() * 5.0;
		
		ImColor statusColor;
		if(console == nullptr) statusColor = ImColor(Colors::blue);
		else if(console->isConnecting()) statusColor = ImColor(Colors::yellow);
		else statusColor = ImColor(Colors::green);

		const char* consoleText;
		if(console == nullptr) consoleText = "No Console";
        else if(console->isConnecting()) consoleText = "Connecting...";
		else consoleText = console->getName().c_str();
			
		backgroundText(consoleText, glm::vec2(width, height), statusColor);
	
		if(console != nullptr){
			auto mapping = console->getMapping();
			if(mapping != nullptr) {
				ImGui::SameLine();
				mapping->gui(height);
			}
		}
		
	}

};
