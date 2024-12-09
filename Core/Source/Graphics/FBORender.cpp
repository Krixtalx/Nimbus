#include "CorePch.h"
#include "FBORender.h"

Nimbus::FBORender::FBORender(uint16_t width, uint16_t height) :
	Framebuffer(width, height),
	_colorAttachment(new Texture(uvec2(width, height), GL_RGBA8, GL_RGBA)),
	_depthAttachment(new Texture(uvec2(width, height), GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL)) {
	glCreateFramebuffers(1, &_id);
	glNamedFramebufferTexture(_id, GL_COLOR_ATTACHMENT0, _colorAttachment->getId(), 0);
	glNamedFramebufferTexture(_id, GL_DEPTH_ATTACHMENT, _depthAttachment->getId(), 0);
}

void Nimbus::FBORender::modifySize(const uint16_t width, const uint16_t height) {
	Framebuffer::modifySize(width, height);
	delete _colorAttachment;
	delete _depthAttachment;

	_colorAttachment = new Texture(uvec2(width, height), GL_RGBA8, GL_RGBA);
	_depthAttachment = new Texture(uvec2(width, height), GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL);
}

void Nimbus::FBORender::bindFramebuffer() {
	glBindFramebuffer(GL_DRAW_BUFFER, _id);
}

void Nimbus::FBORender::bindColorAttachment() const {
	glBindImageTexture(0, _colorAttachment->getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
}

void Nimbus::FBORender::bindDepthAttachment() const {
	glBindImageTexture(0, _depthAttachment->getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_DEPTH24_STENCIL8);
}

uint32_t Nimbus::FBORender::getColorAttachmentId() const {
	return _colorAttachment->getId();
}
