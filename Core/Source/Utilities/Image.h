#pragma once

namespace Nimbus {
	class Image {
	protected:
		std::string					_filename;
		std::vector<unsigned char>	_image;
		unsigned					_width, _height, _depth;

	public:
		Image(const std::string& filename);
		Image(const std::vector<unsigned char>& data);
		Image(unsigned char* image, uint16_t width, uint16_t height, uint8_t depth);
		~Image();

		void flipImageVertically();
		static void flipImageVertically(std::vector<unsigned char>& image, uint16_t width, uint16_t height, uint8_t depth);
		bool saveImage(const std::string& filename) const;

		unsigned char* bits() { return _image.data(); }
		int getDepth() const { return _depth; }
		std::string getFilename() { return _filename; }
		int getHeight() const { return _height; }
		int getWidth() const { return _width; }
	};
}



