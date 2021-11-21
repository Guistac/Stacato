#include <pch.h>

#include "Fonts.h"
#include <imgui.h>

namespace Fonts {

	ImFont* Fonts::robotoRegular12;
	ImFont* Fonts::robotoBold12;
	ImFont* Fonts::robotoLight12;

	ImFont* Fonts::robotoRegular15;
	ImFont* Fonts::robotoBold15;
	ImFont* Fonts::robotoLight15;

	ImFont* Fonts::robotoRegular20;
	ImFont* Fonts::robotoBold20;
	ImFont* Fonts::robotoLight20;

	ImFont* Fonts::robotoRegular42;
	ImFont* Fonts::robotoBold42;
	ImFont* Fonts::robotoLight42;

	void Fonts::load(float scale) {
		ImGuiIO& io = ImGui::GetIO();

		robotoRegular15 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 15.0f * scale);
		robotoBold15 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 15.0f * scale);
		robotoLight15 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 15.0f * scale);

		robotoRegular12 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 12.0f * scale);
		robotoBold12 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 12.0f * scale);
		robotoLight12 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 12.0f * scale);

		robotoRegular20 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 20.0f * scale);
		robotoBold20 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 20.0f * scale);
		robotoLight20 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 20.0f * scale);
		
		robotoRegular42 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 42.0f * scale);
		robotoBold42 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 42.0f * scale);
		robotoLight42 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 42.0f * scale);
	}

}
