#include <pch.h>

#include "Gui.h"

#include <imgui.h>

#include "Fieldbus/EtherCatFieldbus.h"


void gui() {
	static bool startup = true;
	if (startup) {
		startup = false;
		EtherCatFieldbus::updateNetworkInterfaceCardList();
		EtherCatFieldbus::init(EtherCatFieldbus::networkInterfaceCards.front());
	}

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);

	etherCatGui();
}


void quit() {
	EtherCatFieldbus::terminate();
}
