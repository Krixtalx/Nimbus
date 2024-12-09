#include "CorePch.h"
#include "FBOScreenshot.h"

/// [Public methods]

Nimbus::FBOScreenshot::FBOScreenshot(const uint16_t width, const uint16_t height) :
	Framebuffer(width, height) {
	glGenFramebuffers(1, &_id);									// Support for multisampled framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, _id);

	glGenTextures(1, &_colorBufferID);
	glBindTexture(GL_TEXTURE_2D, _colorBufferID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _size.x, _size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Color buffer => FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorBufferID, 0);

	this->checkFramebufferState();

	// Multisampled FBO
	glGenFramebuffers(1, &_multisampledFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _multisampledFBO);

	// New color buffer for multisampled FBO
	glGenRenderbuffers(1, &_mColorBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, _mColorBufferID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, _size.x, _size.y);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Depth buffer for multisampled FBO
	glGenRenderbuffers(1, &_mDepthBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, _mDepthBufferID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, _size.x, _size.y);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Color & depth buffer => Multisampled FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _mColorBufferID);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _mDepthBufferID);

	this->checkFramebufferState();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);				// Default framebuffer
}

Nimbus::FBOScreenshot::~FBOScreenshot() {
	glDeleteTextures(1, &_colorBufferID);
	glDeleteRenderbuffers(1, &_mColorBufferID);
	glDeleteRenderbuffers(1, &_mDepthBufferID);
}

void Nimbus::FBOScreenshot::bindFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, _multisampledFBO);			// Multisampled! Normal FBO is only used as support for this one
}

Nimbus::Image* Nimbus::FBOScreenshot::getImage() const {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _multisampledFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _id);
	glBlitFramebuffer(0, 0, _size.x, _size.y, 0, 0, _size.x, _size.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, _id);

	uint8_t* pixels = (uint8_t*)malloc((int)_size.x * (int)_size.y * 4);
	glReadPixels(0, 0, _size.x, _size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Image* image = new Image(pixels, _size.x, _size.y, 4);
	free(pixels);

	return image;
}

void Nimbus::FBOScreenshot::modifySize(const uint16_t width, const uint16_t height) {
	Framebuffer::modifySize(width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, _id);

	glBindTexture(GL_TEXTURE_2D, _colorBufferID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _size.x, _size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, _multisampledFBO);

	glBindRenderbuffer(GL_RENDERBUFFER, _mColorBufferID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, _size.x, _size.y);

	glBindRenderbuffer(GL_RENDERBUFFER, _mDepthBufferID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, _size.x, _size.y);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Nimbus::FBOScreenshot::saveImage(const std::string& filename) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _multisampledFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _id);
	glBlitFramebuffer(0, 0, _size.x, _size.y, 0, 0, _size.x, _size.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, _id);

	auto* pixels = new std::vector<uint8_t>((int)_size.x * (int)_size.y * 4);
	glReadPixels(0, 0, _size.x, _size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels->data());

	// Correct image orientation
	Image::flipImageVertically(*pixels, _size.x, _size.y, 4);

	// Launch image writing in a thread
	std::thread writeImageThread(&FBOScreenshot::threadedWriteImage, this, pixels, filename + ".png", _size.x, _size.y);
	writeImageThread.detach();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void Nimbus::FBOScreenshot::threadedWriteImage(std::vector<uint8_t>* pixels, const std::string& filename, const uint16_t width, const uint16_t height) {
	const auto image = new Image(pixels->data(), width, height, 4);
	image->saveImage(filename);
	delete pixels;
}

