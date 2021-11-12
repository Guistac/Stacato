#include "StageView.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "GlApplet.h"

#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/VertexColorGL.h>
#include <Magnum/Math/Color.h>


//#include <Magnum/Platform/GLContext.h>
//#include <Magnum/Math/Vector.h>

#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>




class TestGlApplet : public GlApplet {
public:

	//triangle example
	Magnum::GL::Buffer buffer;
	Magnum::GL::Mesh mesh;
	Magnum::GL::Buffer mouseBuffer;
	Magnum::GL::Mesh mouseMesh;
	Magnum::Shaders::VertexColorGL2D shader;

	//primitives example
	Magnum::GL::Mesh cubeMesh;
	Magnum::Shaders::PhongGL phongShader;
	Magnum::Matrix4 _transformation, _projection;
	Magnum::Color3 _color;

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

	//=============== Triangle Example =============

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

	//================= Primitive Examples =================

	GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
	GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

	Trade::MeshData cube = Primitives::cubeSolid();

	GL::Buffer vertices;
	vertices.setData(MeshTools::interleave(cube.positions3DAsArray(),
		cube.normalsAsArray()));

	std::pair<Containers::Array<char>, MeshIndexType> compressed =
		MeshTools::compressIndices(cube.indicesAsArray());
	GL::Buffer indices;
	indices.setData(compressed.first);

	cubeMesh.setPrimitive(cube.primitive())
		.setCount(cube.indexCount())
		.addVertexBuffer(std::move(vertices), 0, Shaders::PhongGL::Position{},
			Shaders::PhongGL::Normal{})
		.setIndexBuffer(std::move(indices), 0, compressed.second);

	_transformation =
		Matrix4::rotationX(30.0_degf) * Matrix4::rotationY(40.0_degf);
	_projection =
		Matrix4::perspectiveProjection(
			35.0_degf, Vector2{ Math::Vector2<int>(getSize().x, getSize().y) }.aspectRatio(), 0.01f, 100.0f) *
		Matrix4::translation(Vector3::zAxis(-10.0f));
	_color = Color3::fromHsv({ 35.0_degf, 1.0f, 1.0f });
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

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		_color = Color3::fromHsv({ _color.hue() + 50.0_degf, 1.0f, 1.0f });
	}

	static glm::vec2 previousMousePosition = ImGui::GetMousePos();
	glm::vec2 currentMousePosition = ImGui::GetMousePos();
	glm::vec2 dragDelta = currentMousePosition - previousMousePosition;
	previousMousePosition = currentMousePosition;

	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		glm::vec2 rotationDragDelta = dragDelta * glm::vec2(0.005);
		_transformation = Matrix4::rotationX(Rad{ rotationDragDelta.y }) * _transformation * Matrix4::rotationY(Rad{ rotationDragDelta.x });
	}

	phongShader.setLightPositions({ {1.4f, 1.0f, 0.75f, 0.0f} })
		.setDiffuseColor(_color)
		.setAmbientColor(Color3::fromHsv({ _color.hue(), 1.0f, 0.3f }))
		.setTransformationMatrix(_transformation)
		.setNormalMatrix(_transformation.normalMatrix())
		.setProjectionMatrix(_projection)
		.draw(cubeMesh);
}





void stageView() {

	glm::vec2 canvasSize = ImGui::GetContentRegionAvail();
	
	ImGui::InvisibleButton("##MagnumCanvas", canvasSize);
	
	static TestGlApplet applet(canvasSize);
	
	if (applet.getSize() != canvasSize) applet.setFramebufferSize(canvasSize);

	applet.update();	

	ImGui::GetWindowDrawList()->AddImage((ImTextureID)applet.getFrameBufferTextureId(), ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), glm::vec2(0, 1), glm::vec2(1, 0));
}