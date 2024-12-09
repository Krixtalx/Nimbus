#pragma once

#include "Texture.h"
#include "Utilities/Singleton.h"

class TextureList: public Nimbus::Singleton<TextureList>
{
	friend Singleton;

private:
	struct ColorHash {
		std::size_t operator()(const vec4& color) const
		{
			const glm::ivec4 iColor = color * 256.0f;
			return static_cast<size_t>(iColor.x + iColor.y * 256 + iColor.z * 512 + iColor.w + 1024);
		}
	};

	struct ColorEqual {
		bool operator()(const vec4& c1, const vec4& c2) const
		{
			return glm::distance(c1, c2) < glm::epsilon<float>();
		}
	};

private:
	std::unordered_map<vec4, Nimbus::Texture*, ColorHash, ColorEqual> _colorTexture;
	std::unordered_map<std::string, Nimbus::Texture*>		_imageTexture;

private:
	TextureList();

public:
	virtual ~TextureList();

	Nimbus::Texture* isTextureAvailable(const vec4& color);
	Nimbus::Texture* isTextureAvailable(const std::string& path);
	void saveTexture(const vec4& color, Nimbus::Texture* texture);
	void saveTexture(const std::string& path, Nimbus::Texture* texture);

	std::vector<vec4> getAllColorTex() const;
	std::vector<std::string> getAllImageTex() const;
};

