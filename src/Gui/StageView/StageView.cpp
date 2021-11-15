#include "StageView.h"

#include <imgui.h>
#include <imgui_internal.h>

#ifdef STACATO_USE_MAGNUM_ENGINE
	#include <Magnum/GL/Context.h>
	#include "StageViewApplet.h"
#endif

namespace StageView {

	void draw() {

		glm::vec2 canvasSize = ImGui::GetContentRegionAvail();

		ImGui::InvisibleButton("##MagnumCanvas", canvasSize);

#ifdef STACATO_USE_MAGNUM_ENGINE

		if (Magnum::GL::Context::hasCurrent()) {

			static TestGlApplet* glApplet = nullptr;
			if (!glApplet) glApplet = new TestGlApplet(canvasSize);

			if (glApplet->getSize() != canvasSize) glApplet->setFramebufferSize(canvasSize);

			glApplet->update();

			ImGui::GetWindowDrawList()->AddImage((ImTextureID)glApplet->getFrameBufferTextureId(), ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), glm::vec2(0, 1), glm::vec2(1, 0));
		}
#endif

	}

}