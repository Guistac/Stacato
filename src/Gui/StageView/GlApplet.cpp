#include <pch.h>

#include "GlApplet.h"

#ifdef STACATO_USE_MAGNUM_ENGINE

#include <Magnum/GL/TextureFormat.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/GL/DefaultFramebuffer.h>

GlApplet::GlApplet(glm::vec2 s) : size(s) {
	Logger::critical("Constructed GlApplet");
	viewPortRange = Magnum::Range2Di(Magnum::Vector2i(0, 0), Magnum::Vector2i(size.x, size.y));
	frameBuffer = new Magnum::GL::Framebuffer(viewPortRange);
	setFramebufferSize(s);
}

void GlApplet::setFramebufferSize(glm::vec2 s) {
	Magnum::Math::Vector2<int> sizeInt(size.x, size.y);
	texture.setStorage(1, Magnum::GL::TextureFormat::RGBA8, sizeInt);
	depthStencil.setStorage(Magnum::GL::RenderbufferFormat::Depth24Stencil8, sizeInt);
	frameBuffer->attachTexture(Magnum::GL::Framebuffer::ColorAttachment{ 0 }, texture, 0);
	frameBuffer->attachRenderbuffer(Magnum::GL::Framebuffer::BufferAttachment::DepthStencil, depthStencil);
}

GLuint GlApplet::getFrameBufferTextureId() {
	return texture.id();
}

glm::vec2 GlApplet::getSize() {
	return size;
}

void GlApplet::update() {
	frameBuffer->clear(Magnum::GL::FramebufferClear::Color | Magnum::GL::FramebufferClear::Depth);
	frameBuffer->bind();
	updateEvent();
	Magnum::GL::defaultFramebuffer.bind();
}


#endif