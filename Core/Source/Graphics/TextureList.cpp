#include "CorePch.h"
#include "TextureList.h"

TextureList::TextureList() {}

TextureList::~TextureList() {
	for (const auto& pair : _colorTexture) {
		delete pair.second;
	}

	for (const auto& pair : _imageTexture) {
		delete pair.second;
	}
}

Nimbus::Texture* TextureList::isTextureAvailable(const vec4& color) {
	const auto it = _colorTexture.find(color);

	return it != _colorTexture.end() ? it->second : nullptr;
}

Nimbus::Texture* TextureList::isTextureAvailable(const std::string& path) {
	const auto it = _imageTexture.find(path);

	return it != _imageTexture.end() ? it->second : nullptr;
}

void TextureList::saveTexture(const vec4& color, Nimbus::Texture* texture) {
	_colorTexture[color] = texture;
}

void TextureList::saveTexture(const std::string& path, Nimbus::Texture* texture) {
	_imageTexture[path] = texture;
}

std::vector<vec4> TextureList::getAllColorTex() const {
	std::vector<vec4> tex;

	for (auto& colorTexPair : _colorTexture) {
		tex.push_back(colorTexPair.first);
	}

	return tex;
}

std::vector<std::string> TextureList::getAllImageTex() const {
	std::vector<std::string> tex;

	for (auto& imgTex : _imageTexture) {
		tex.push_back(imgTex.first);
	}

	return tex;
}
