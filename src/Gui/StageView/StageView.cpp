#include "StageView.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "GlApplet.h"



class TestGlApplet : public GlApplet {
public:

	Magnum::GL::Buffer buffer;
	Magnum::GL::Mesh mesh;

	Magnum::GL::Buffer mouseBuffer;
	Magnum::GL::Mesh mouseMesh;

	Magnum::Shaders::VertexColorGL2D shader;

	TestGlApplet(glm::vec2 s);
	virtual void updateEvent();
};


struct TriangleVertex {
	Magnum::Vector2 position;
	Magnum::Color3 color;
};

TestGlApplet::TestGlApplet(glm::vec2 s) : GlApplet(s) {
	using namespace Magnum;
	using namespace Math::Literals;
	const TriangleVertex data[]{
		{{-0.5f, -0.5f}, 0xff0000_rgbf},    //Left vertex, red color
		{{ 0.5f, -0.5f}, 0x00ff00_rgbf},    //Right vertex, green color
		{{ 0.0f,  0.5f}, 0x0000ff_rgbf}     //Top vertex, blue color
	};
	buffer.setData(data);
	mesh.setPrimitive(GL::MeshPrimitive::Triangles)
		.setCount(3)
		.addVertexBuffer(buffer, 0,
			Shaders::VertexColorGL2D::Position{},
			Shaders::VertexColorGL2D::Color3{});
}

void TestGlApplet::updateEvent() {

	glm::vec2 rectMin = ImGui::GetItemRectMin();
	glm::vec2 rectMax = ImGui::GetItemRectMax();
	glm::vec2 mousePos = ImGui::GetMousePos();
	glm::vec2 mouseBufferpos;
	mouseBufferpos.x = (mousePos.x - rectMin.x) / (rectMax.x - rectMin.x);
	mouseBufferpos.y = (mousePos.y - rectMin.y) / (rectMax.y - rectMin.y);
	mouseBufferpos.x = mouseBufferpos.x * 2.0 - 1.0;
	mouseBufferpos.y = mouseBufferpos.y * -2.0 + 1.0;

	using namespace Magnum;
	using namespace Math::Literals;

	float dist = 0.1;
	const TriangleVertex mouse[]{
		{{mouseBufferpos.x - dist, mouseBufferpos.y - dist}, 0xffffff_rgbf},    //Left vertex, red color
		{{mouseBufferpos.x + dist, mouseBufferpos.y - dist}, 0xffffff_rgbf},    //Right vertex, green color
		{{mouseBufferpos.x,  mouseBufferpos.y + dist}, 0xffffff_rgbf}     //Top vertex, blue color
	};
	mouseBuffer.setData(mouse);

	mouseMesh.setPrimitive(GL::MeshPrimitive::Triangles)
		.setCount(3)
		.addVertexBuffer(mouseBuffer, 0,
			Shaders::VertexColorGL2D::Position{},
			Shaders::VertexColorGL2D::Color3{});

	shader.draw(mesh);
	shader.draw(mouseMesh);
}




void stageView() {

	glm::vec2 canvasSize = ImGui::GetContentRegionAvail();
	
	ImGui::InvisibleButton("##MagnumCanvas", canvasSize);
	
	static TestGlApplet applet(canvasSize);
	
	if (applet.getSize() != canvasSize) applet.setFramebufferSize(canvasSize);

	applet.update();	

	ImGui::GetWindowDrawList()->AddImage((ImTextureID)applet.getFrameBufferTextureId(), ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), glm::vec2(0, 1), glm::vec2(1, 0));
}