#pragma once

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

#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Trade/MeshData.h>

class GlApplet {
public:

	GlApplet(glm::vec2 s);
	void setFramebufferSize(glm::vec2 s);
	GLuint getFrameBufferTextureId();
	glm::vec2 getSize();
	void update();

private:

	Magnum::Range2Di viewPortRange;
	glm::vec2 size;
	Magnum::GL::Framebuffer* frameBuffer;
	Magnum::GL::Texture2D texture;
	Magnum::GL::Renderbuffer depthStencil;

	virtual void updateEvent() = 0;
};