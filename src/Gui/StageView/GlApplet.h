#pragma once

#ifdef STACATO_USE_MAGNUM_ENGINE

#include <Magnum/GL/Texture.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/Framebuffer.h>

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

#endif