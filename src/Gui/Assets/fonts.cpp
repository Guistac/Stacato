#include <pch.h>

#include "Fonts.h"
#include <imgui.h>

namespace Fonts {

	ImFont* sansRegular12;
	ImFont* sansBold12;
	ImFont* sansLight12;

	ImFont* sansRegular15;
	ImFont* sansBold15;
	ImFont* sansLight15;

	ImFont* sansRegular20;
	ImFont* sansBold20;
	ImFont* sansLight20;

	ImFont* sansRegular42;
	ImFont* sansBold42;
	ImFont* sansLight42;

	ImFont* mono15;

	void load(float scale) {
		ImGuiIO& io = ImGui::GetIO();

		ImFontConfig conf;
		conf.OversampleH = 4;
		conf.OversampleV = 4;
		
		sansRegular15 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 15.0 * scale, &conf);
		sansBold15 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 15.0f * scale, &conf);
		sansLight15 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 15.0f * scale, &conf);
		mono15 = io.Fonts->AddFontFromFileTTF("fonts/FiraCode-Retina.ttf", 15.0f * scale, &conf);
		
		sansRegular12 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 12.0f * scale, &conf);
		sansBold12 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 12.0f * scale, &conf);
		sansLight12 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 12.0f * scale, &conf);

		sansRegular20 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 20.0f * scale, &conf);
		sansBold20 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 20.0f * scale, &conf);
		sansLight20 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 20.0f * scale, &conf);
		
		sansRegular42 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Regular.ttf", 42.0f * scale, &conf);
		sansBold42 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Bold.ttf", 42.0f * scale, &conf);
		sansLight42 = io.Fonts->AddFontFromFileTTF("fonts/OpenSans-Light.ttf", 42.0f * scale, &conf);
		
	}

}
