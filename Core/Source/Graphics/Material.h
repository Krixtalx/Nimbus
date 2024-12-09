#pragma once

#include "Texture.h"
#include "Shaders/ShaderEnums.h"

namespace Nimbus {
	class RenderingShader;

	class Material {
		friend class DetailsWindow;
		friend class MaterialList;

	public:
		struct MaterialDescription {
			std::string					_rootFolder;
			std::vector<std::string>	_textureImage;
			std::vector<vec4>			_textureColor;
			float						_ns = 0;

			MaterialDescription() : _textureImage(Texture::NUM_TEXTURE_TYPES), _textureColor(Texture::NUM_TEXTURE_TYPES) {}

			void setRootFolder(const std::string& path) {
				_rootFolder = path;
			}

			void setTextureColor(const Texture::TextureLayer texture, const vec4& color) {
				_textureColor[texture] = color;
			}

			void setTexturePath(const Texture::TextureLayer texture, const std::string& path) {
				_textureImage[texture] = path;
			}
		};

		//int					_band = 0; //Used for multispectral and hyperspectral data
		int					_activeSubroutine[ShaderEnum::NUM_SHADER_TYPES] = { 0,0,0,0 };
	protected:
		std::string _name;
		RenderingShader* _associatedShader = nullptr;
		Texture* _texture[Texture::NUM_TEXTURE_TYPES]{};
		float				_displacementFactor;
		float				_shininess;
		std::vector<std::string> _enabledSubroutines[ShaderEnum::NUM_SHADER_TYPES];
	public:
		Material(const std::string& name, RenderingShader* shader);
		Material(const Material& material) = delete;
		Material& operator=(const Material& material) = delete;

		void applyMaterial(const Texture::RenderTextureRequirements* renderRequirements = nullptr) const;
		MaterialDescription getMaterialDescription(const std::string& rootFolder) const;
		RenderingShader* getAssociatedShader() const;
		std::string getName() const;

		void setDisplacementFactor(float dispFactor);
		void setShininess(float shininess);
		void setTexture(Texture::TextureLayer textureLayer, Texture* texture);
		void setSubroutine(const ShaderEnum::VertexSubroutines& subroutine, const std::string& functionName);
		void setSubroutine(const ShaderEnum::GeometrySubroutines& subroutine, const std::string& functionName);
		void setSubroutine(const ShaderEnum::FragmentSubroutines& subroutine, const std::string& functionName);
	};
}
