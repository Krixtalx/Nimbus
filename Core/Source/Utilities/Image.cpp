#include "CorePch.h"
#include "Utilities/Image.h"

#include "lodepng.h"

Nimbus::Image::Image(const std::string& filename) :_filename(filename), _depth(4)				// PNG depth
{
	const unsigned error = lodepng::decode(_image, _width, _height, filename);

	if (error) {
		spdlog::error("The image {} couldn't be loaded by lodepng", filename);
		_width = _height = _depth = 0;
	} else
		Nimbus::Image::flipImageVertically(_image, _width, _height, _depth);				// By default it's flipped
}

Nimbus::Image::Image(const std::vector<unsigned char>& data) {
	const unsigned error = lodepng::decode(_image, _width, _height, data.data(), data.size());

	if (error) {
		spdlog::error("The image couldn't be loaded by lodepng");
		_width = _height = _depth = 0;
	} else
		Nimbus::Image::flipImageVertically(_image, _width, _height, _depth);				// By default it's flipped
}


Nimbus::Image::Image(unsigned char* image, const uint16_t width, const uint16_t height, const uint8_t depth) : _width(width), _height(height), _depth(depth) {
	if (image) {
		const int size = width * height * depth;
		_image = std::vector<unsigned char>(image, image + size);
	} else {
		spdlog::error("Empty image!");
		_width = _height = _depth = 0;
	}
}

Nimbus::Image::~Image() = default;

void Nimbus::Image::flipImageVertically(std::vector<unsigned char>& image, const uint16_t width, const uint16_t height, const uint8_t depth) {
	const int rowSize = width * depth;
	unsigned char* bits = image.data();
	const auto tempBuffer = new unsigned char[rowSize];

	for (int i = 0; i < (height / 2); ++i) {
		unsigned char* source = bits + i * rowSize;
		unsigned char* target = bits + (height - i - 1) * rowSize;

		memcpy(tempBuffer, source, rowSize);					// Swap with help of temporary buffer
		memcpy(source, target, rowSize);
		memcpy(target, tempBuffer, rowSize);
	}

	delete[] tempBuffer;
}

bool Nimbus::Image::saveImage(const std::string& filename) const {
	std::vector<unsigned char> result;
	const unsigned error = lodepng::encode(result, this->_image, this->_width, this->_height);

	if (!error) {
		lodepng::save_file(result, filename);
		return true;
	}

	return false;
}

void Nimbus::Image::flipImageVertically() {
	Image::flipImageVertically(_image, _width, _height, _depth);
}