#include <pch.h>

#include "Fonts.h"
#include <imgui.h>

namespace Fonts {

	ImFont* robotoRegular12;
	ImFont* robotoBold12;
	ImFont* robotoLight12;

	ImFont* robotoRegular15;
	ImFont* robotoBold15;
	ImFont* robotoLight15;

	ImFont* robotoRegular20;
	ImFont* robotoBold20;
	ImFont* robotoLight20;

	ImFont* robotoRegular42;
	ImFont* robotoBold42;
	ImFont* robotoLight42;

	void load(float scale) {
		ImGuiIO& io = ImGui::GetIO();

		ImFontConfig conf;
		conf.OversampleH = 4;
		conf.OversampleV = 4;
		
		robotoRegular15 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 15.0 * scale, &conf);
		robotoBold15 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 15.0f * scale, &conf);
		robotoLight15 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 15.0f * scale, &conf);

		robotoRegular12 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 12.0f * scale, &conf);
		robotoBold12 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 12.0f * scale, &conf);
		robotoLight12 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 12.0f * scale, &conf);

		robotoRegular20 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 20.0f * scale, &conf);
		robotoBold20 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 20.0f * scale, &conf);
		robotoLight20 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 20.0f * scale, &conf);
		
		robotoRegular42 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 42.0f * scale, &conf);
		robotoBold42 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 42.0f * scale, &conf);
		robotoLight42 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 42.0f * scale, &conf);
		
	}

}
