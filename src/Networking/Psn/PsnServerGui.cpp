#include <pch.h>

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/HelpMarker.h"

#include "PsnServer.h"

#include "Gui/Utilities/CustomWidgets.h"

void PsnServer::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Connection")){
		//networkGui();
		ImGui::EndTabItem();
	}
}
