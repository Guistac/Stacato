#include <pch.h>

#include <imgui.h>

#include "StageViewApplet.h"

#ifdef STACATO_USE_MAGNUM_ENGINE

#include <Magnum/GL/Renderer.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

TestGlApplet::TestGlApplet(glm::vec2 s) : GlApplet(s) {

	using namespace Magnum;
	using namespace Math::Literals;

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

	struct TriangleVertex {
		Magnum::Vector2 position;
		Magnum::Color3 color;
	};

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

	shader.draw(mouseMesh);

	if (ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		_color = Color3::fromHsv({ _color.hue() + 50.0_degf, 1.0f, 1.0f });
	}

	static glm::vec2 previousMousePosition = ImGui::GetMousePos();
	glm::vec2 currentMousePosition = ImGui::GetMousePos();
	glm::vec2 dragDelta = currentMousePosition - previousMousePosition;
	previousMousePosition = currentMousePosition;

	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
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

#endif