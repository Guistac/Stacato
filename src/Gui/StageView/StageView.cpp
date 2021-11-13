#include "StageView.h"

#include <imgui.h>
#include <imgui_internal.h>



#include "StageViewApplet.h"

namespace StageView {

	void draw() {

		glm::vec2 canvasSize = ImGui::GetContentRegionAvail();

		ImGui::InvisibleButton("##MagnumCanvas", canvasSize);

		static TestGlApplet* glApplet = nullptr;
		if (!glApplet) glApplet = new TestGlApplet(canvasSize);

		if (glApplet->getSize() != canvasSize) glApplet->setFramebufferSize(canvasSize);

		glApplet->update();

		ImGui::GetWindowDrawList()->AddImage((ImTextureID)glApplet->getFrameBufferTextureId(), ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), glm::vec2(0, 1), glm::vec2(1, 0));
	}

}