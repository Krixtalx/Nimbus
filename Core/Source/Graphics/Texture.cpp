#include "CorePch.h"
#include "Texture.h"

#include "Shaders/ShaderProgram.h"
#include "Utilities/Image.h"

// [Static variables initialization]

const uint32_t Nimbus::Texture::MAG_FILTER = GL_LINEAR;
const uint32_t Nimbus::Texture::MIN_FILTER = GL_LINEAR_MIPMAP_NEAREST;
const uint32_t Nimbus::Texture::WRAP_S = GL_MIRRORED_REPEAT;
const uint32_t Nimbus::Texture::WRAP_T = GL_MIRRORED_REPEAT;
const uint32_t Nimbus::Texture::WRAP_R = GL_MIRRORED_REPEAT;

const std::unordered_map<uint16_t, std::string> Nimbus::Texture::SHADER_VARIABLE = std::unordered_map<uint16_t, std::string>({
		{Texture::KAD, "texKdSampler"},
		{Texture::METALLIC, "texKsSampler"},
		{Texture::SEMI_TRANSPARENT, "texSemiTransparentSampler"},
		{Texture::BUMP, "texBumpSampler"},
		{Texture::HEIGHT, "texDisplacementSampler"},
		{Texture::ROUGHNESS, "texRoughnessSampler"}
	});

/// [Public methods]

/** glTexImage2D:
* Target. Type of desired texture: GL_TEXTURE_2D, GL_PROXY_TEXTURE_2D...
* Level:  Level of Detail. Level 0 is the base image. Level n is n-reduced image with mipmap algorithm
* Internal Format:  Number of texture components
* Width
* Height
* Border: must be 0
* Format: Format of pixel information. GL_RED, GL_RG, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_RED_INTEGER...
* Type: Type of data of pixel information. GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT...
* Data: Image pixels
*/

Nimbus::Texture::Texture(Image* image, const uint32_t wrapS, const uint32_t wrapT, const uint32_t minFilter, const uint32_t magFilter, const bool swizzleG, const bool swizzleB)
	: _color(.0f), _filename(image->getFilename()) {
	const unsigned char* bits = image->bits();
	if (image and !bits) {
		throw std::runtime_error("Failed to generate texture from image!: " + image->getFilename());
	}

	_width = image->getWidth();
	_height = image->getHeight();
	const uint32_t depthID[] = { GL_RED, GL_RED, GL_RGB, GL_RGBA };

	glGenTextures(1, &_id);
	glBindTexture(GL_TEXTURE_2D, _id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

	if (swizzleG) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
	if (swizzleB) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
	if (swizzleG or swizzleB) glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


	glTexImage2D(GL_TEXTURE_2D, 0, depthID[image->getDepth() - 1], _width, _height, 0, depthID[image->getDepth() - 1], GL_UNSIGNED_BYTE, bits);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Nimbus::Texture::Texture(const uvec2 textureSize, const int32_t internalFormat, const GLenum format, const uint32_t wrapS, const uint32_t wrapT, const uint32_t minFilter, const uint32_t magFilter) {
	_width = textureSize.x;
	_height = textureSize.y;

	glGenTextures(1, &_id);
	glBindTexture(GL_TEXTURE_2D, _id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

	if (internalFormat == GL_DEPTH24_STENCIL8)
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, format, GL_UNSIGNED_INT_24_8, nullptr);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, format, GL_UNSIGNED_BYTE, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Nimbus::Texture::Texture(const vec4& color)
	: _color(color) {
	constexpr int width = 1, height = 1;
	const unsigned char image[] = {
		static_cast<unsigned char>(255.0f * color.x), static_cast<unsigned char>(255.0f * color.y),
		static_cast<unsigned char>(255.0f * color.z), static_cast<unsigned char>(255.0f * color.a)
	};

	glGenTextures(1, &_id);
	glBindTexture(GL_TEXTURE_2D, _id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MIN_FILTER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAG_FILTER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WRAP_S);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WRAP_T);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Nimbus::Texture::~Texture() {
	glDeleteTextures(1, &_id);
}

void Nimbus::Texture::applyTexture(Nimbus::ShaderProgram* shader, const TextureLayer textureLayer) const {
	const int textureID = textureLayer;

	if (_id < UINT32_MAX) {
		shader->setUniform(SHADER_VARIABLE.at(textureID), textureID);
		glActiveTexture(GL_TEXTURE0 + textureID);
		glBindTexture(GL_TEXTURE_2D, _id);
	}
}

void Nimbus::Texture::applyTexture(Nimbus::ShaderProgram* shader, const int32_t id, const std::string& shaderVariable) const {
	if (_id < UINT32_MAX) {
		shader->setUniform(shaderVariable, id);
		glActiveTexture(GL_TEXTURE0 + id);
		glBindTexture(GL_TEXTURE_2D, _id);
	}
}