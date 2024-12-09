#include "CorePch.h"
#include "Framebuffer.h"

Nimbus::Framebuffer::Framebuffer(const uint16_t width, const uint16_t height) :
	_id(0), _size(width, height) {
}

Nimbus::Framebuffer::~Framebuffer() {
	glDeleteFramebuffers(1, &_id);
}

void Nimbus::Framebuffer::modifySize(const uint16_t width, const uint16_t height) {
	_size = vec2(width, height);
}

bool Nimbus::Framebuffer::checkFramebufferStatus() {
	const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	return status == GL_FRAMEBUFFER_COMPLETE;
}

void Nimbus::Framebuffer::checkFramebufferState() {
	const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer could not be created!");
	}
}
