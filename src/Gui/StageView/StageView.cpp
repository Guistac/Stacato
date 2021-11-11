#include "StageView.h"

#include <imgui.h>

#include <Magnum/Platform/GLContext.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Shaders/VertexColorGL.h>

#include <Magnum/GL/Texture.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Math/Vector.h>
#include <Magnum/GL/Framebuffer.h>

bool firstFrame = true;

void stageView() {

	using namespace Magnum;

	static glm::vec2 canvasSize = ImGui::GetContentRegionAvail();
	glm::vec2 actualCanvasSize = ImGui::GetContentRegionAvail();

	static Magnum::GL::Texture2D color;
	static Magnum::GL::Renderbuffer depthStencil;
	static Magnum::Math::Vector2<int> size(canvasSize.x, canvasSize.y);
	static GL::Framebuffer framebuffer{ {{}, size} };

	if (firstFrame || canvasSize != actualCanvasSize) {
		firstFrame = false;
		canvasSize = actualCanvasSize;
		color.setStorage(1, GL::TextureFormat::RGBA8, size);
		depthStencil.setStorage(GL::RenderbufferFormat::Depth24Stencil8, size);
		
		framebuffer.attachTexture(GL::Framebuffer::ColorAttachment{ 0 }, color, 0);
		framebuffer.attachRenderbuffer(GL::Framebuffer::BufferAttachment::DepthStencil, depthStencil);
	}

	//bind drawing framebuffer
	framebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth).bind();

	// Setup the colored triangle
	using namespace Math::Literals;

	struct TriangleVertex {
		Vector2 position;
		Color3 color;
	};
	const TriangleVertex data[]{
		{{-0.5f, -0.5f}, 0xff0000_rgbf},    //Left vertex, red color
		{{ 0.5f, -0.5f}, 0x00ff00_rgbf},    //Right vertex, green color
		{{ 0.0f,  0.5f}, 0x0000ff_rgbf}     //Top vertex, blue color
	};

	GL::Buffer buffer;
	buffer.setData(data);


	GL::Mesh mesh;
	mesh.setPrimitive(GL::MeshPrimitive::Triangles)
		.setCount(3)
		.addVertexBuffer(buffer, 0,
			Shaders::VertexColorGL2D::Position{},
			Shaders::VertexColorGL2D::Color3{});

	Shaders::VertexColorGL2D shader;

	shader.draw(mesh);

	/* Switch back to the default framebuffer */
	GL::defaultFramebuffer.bind();


	ImGui::Image((ImTextureID)color.id(), canvasSize, glm::vec2(0,1), glm::vec2(1,0), glm::vec4(1.0, 1.0, 1.0, 1.0));


}