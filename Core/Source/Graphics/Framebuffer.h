#pragma once

namespace Nimbus {
	class Framebuffer {
	protected:
		uint32_t	_id;
		ivec2	_size;

		void checkFramebufferState();

	public:
		Framebuffer(uint16_t width, uint16_t height);
		virtual ~Framebuffer();

		virtual uint32_t getId() const { return _id; }
		ivec2 getSize() const { return _size; }

		virtual void bindFramebuffer() = 0;
		virtual void modifySize(uint16_t width, uint16_t height);

		static bool checkFramebufferStatus();
	};
}

