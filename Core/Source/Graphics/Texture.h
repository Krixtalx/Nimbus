#pragma once
#include "glad/glad.h"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

namespace Nimbus {
	class Image;
	class ShaderProgram;

	class Texture {
	public:
		enum TextureLayer {
			KAD, AO, BUMP, HEIGHT, SEMI_TRANSPARENT, BLOOM, ROUGHNESS, METALLIC,
			NUM_TEXTURE_TYPES
		};
		inline static const std::string TEXTURE_LAYER_IDS[NUM_TEXTURE_TYPES] = {
			"KAD", "AO", "BUMP", "HEIGHT", "SEMI_TANSPARENT", "BLOOM", "ROUGHNESS", "METALLIC"
		};

		struct RenderTextureRequirements {
			uint16_t _applyTextureLayer;

			RenderTextureRequirements() : _applyTextureLayer(UINT16_MAX) {}
			bool isLayerActive(const TextureLayer layer) const { return (_applyTextureLayer >> layer) & 1; }
			void activeLayer(const TextureLayer layer) { _applyTextureLayer |= (1 << layer); }
			void deactivateLayer(const TextureLayer layer) { _applyTextureLayer &= ~(1 << layer); }
			void toggle(const TextureLayer layer) { _applyTextureLayer ^= (1 << layer); }
		};

	protected:
		const static uint32_t MIN_FILTER;
		const static uint32_t MAG_FILTER;
		const static uint32_t WRAP_S;
		const static uint32_t WRAP_T;
		const static uint32_t WRAP_R;
		const static std::unordered_map<uint16_t, std::string> SHADER_VARIABLE;

	protected:
		uint32_t	_id = UINT32_MAX;
		vec4 _color;
		std::string _filename;
		unsigned	_width;
		unsigned	_height;

	public:
		Texture(Image* image, uint32_t wrapS = WRAP_S, uint32_t wrapT = WRAP_T, uint32_t minFilter = MIN_FILTER, uint32_t magFilter = MAG_FILTER, bool swizzleG = false, bool swizzleB = false);
		Texture(uvec2 textureSize, int32_t internalFormat, GLenum format, uint32_t wrapS = WRAP_S, uint32_t wrapT = WRAP_T, uint32_t minFilter = MIN_FILTER, uint32_t magFilter = MAG_FILTER);
		Texture(const vec4& color);
		virtual ~Texture();

		void applyTexture(Nimbus::ShaderProgram* shader, TextureLayer textureLayer) const;
		void applyTexture(Nimbus::ShaderProgram* shader, int32_t id, const std::string& shaderVariable) const;

		vec4 getColor() const { return _color; }
		std::string getFilename() { return _filename; }
		uint32_t getId() const { return _id; };
		unsigned getWidth() const { return _width; };
		unsigned getHeight() const { return _height; };
		vec2 getSize() const { return { _width, _height }; };
	};
}