#pragma once
#include "Framebuffer.h"
#include "Texture.h"

namespace Nimbus {
	class FBORender : public Framebuffer {
		Texture* _colorAttachment;
		Texture* _depthAttachment;

	public:
		FBORender(uint16_t width, uint16_t height);
		void modifySize(uint16_t width, uint16_t height) override;
		void bindFramebuffer() override;
		void bindColorAttachment() const;
		void bindDepthAttachment() const;
		uint32_t getColorAttachmentId() const;
	};
}
