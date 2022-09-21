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
        glm::vec2 boxSize(ImGui::GetTextLineHeight() * 4.0, height);
        if(console == nullptr) backgroundText("Offline", boxSize, Colors::blue);
        else if(console->isConnecting()) backgroundText("Connecting", boxSize, Colors::yellow);
        else if(auto mapping = console->getMapping()) mapping->gui(height);
        else backgroundText("No Mapping", boxSize, Colors::red);
	}

};
