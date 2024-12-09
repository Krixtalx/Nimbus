#pragma once

#include "Framebuffer.h"
#include "Utilities/Image.h"

namespace Nimbus {
	class FBOScreenshot : public Framebuffer {
	protected:
		uint32_t _multisampledFBO, _colorBufferID;
		uint32_t _mColorBufferID, _mDepthBufferID;

	protected:
		void threadedWriteImage(std::vector<uint8_t>* pixels, const std::string& filename, uint16_t width, uint16_t height);

	public:
		FBOScreenshot(uint16_t width, uint16_t height);
		~FBOScreenshot() override;

		uint32_t getId() const override { return _multisampledFBO; }
		Image* getImage() const;

		void bindFramebuffer() override;
		void modifySize(uint16_t width, uint16_t height) override;
		bool saveImage(const std::string& filename);
	};
}



