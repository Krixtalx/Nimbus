#include "CorePch.h"
#include "Material.h"

#include "Shaders/RenderingShader.h"


Nimbus::Material::Material(const std::string& name, RenderingShader* shader) : _name(name), _associatedShader(shader), _displacementFactor(.01f), _shininess(1.0f) {
	for (auto& texture : _texture) {
		texture = nullptr;
	}
	for (int shaderType = 0; shaderType < ShaderEnum::NUM_SHADER_TYPES; shaderType++) {
		_enabledSubroutines[shaderType].resize(ShaderEnum::SubroutineUniformNames[shaderType].size());
		for (size_t subroutine = 0; subroutine < ShaderEnum::SubroutineUniformNames[shaderType].size(); ++subroutine) {
			_enabledSubroutines[shaderType][subroutine] = ShaderEnum::SubroutineUniformNames[shaderType][subroutine][0];
		}
	}
}

void Nimbus::Material::applyMaterial(const Texture::RenderTextureRequirements* renderRequirements) const {
	if (renderRequirements)
		for (int textureIdx = 0; textureIdx < Texture::KAD + 1; ++textureIdx) {
			const auto textureLayer = static_cast<Texture::TextureLayer>(textureIdx);
			if (renderRequirements->isLayerActive(textureLayer) && _texture[textureLayer]) {
				_texture[textureLayer]->applyTexture(_associatedShader, textureLayer);
			}
		}
	for (uint8_t shaderType = 0; shaderType < ShaderEnum::NUM_SHADER_TYPES-1; ++shaderType) {
		for (uint8_t subroutine = 0; subroutine < _enabledSubroutines[shaderType].size(); ++subroutine) {
			if (_associatedShader->support(shaderType, subroutine)) {
				_associatedShader->setSubroutineUniform(shaderType, subroutine, _enabledSubroutines[shaderType][subroutine]);
			}
		}
	}
	if (_associatedShader->support(ShaderEnum::FRAGMENT_SHADER, ShaderEnum::LIGHTING))
		_associatedShader->setUniform("shininess", _shininess);
	_associatedShader->applyActiveSubroutines();
}

Nimbus::Material::MaterialDescription Nimbus::Material::getMaterialDescription(const std::string& rootFolder) const {
	MaterialDescription description{};

	description.setRootFolder(rootFolder);
	description._ns = _shininess;

	for (int textureLayer = 0; textureLayer < Texture::NUM_TEXTURE_TYPES; textureLayer += 1) {
		const auto textureType = static_cast<Texture::TextureLayer>(textureLayer);

		if (_texture[textureLayer]) {
			description.setTexturePath(textureType, _texture[textureLayer]->getFilename());
			description.setTextureColor(textureType, _texture[textureLayer]->getColor());
		} else {
			description.setTexturePath(textureType, "");
			description.setTextureColor(textureType, vec4(.0f));
		}
	}

	return description;
}

Nimbus::RenderingShader* Nimbus::Material::getAssociatedShader() const {
	return _associatedShader;
}

std::string Nimbus::Material::getName() const {
	return _name;
}

void Nimbus::Material::setDisplacementFactor(const float dispFactor) {
	_displacementFactor = dispFactor;
}

void Nimbus::Material::setShininess(const float shininess) {
	_shininess = shininess;
}

void Nimbus::Material::setTexture(const Texture::TextureLayer textureLayer, Texture* texture) {
	_texture[textureLayer] = texture;
}

void Nimbus::Material::setSubroutine(const ShaderEnum::VertexSubroutines& subroutine, const std::string& functionName) {
	_enabledSubroutines[ShaderEnum::VERTEX_SHADER][subroutine] = functionName;
}

void Nimbus::Material::setSubroutine(const ShaderEnum::GeometrySubroutines& subroutine, const std::string& functionName) {
	_enabledSubroutines[ShaderEnum::GEOMETRY_SHADER][subroutine] = functionName;
}

void Nimbus::Material::setSubroutine(const ShaderEnum::FragmentSubroutines& subroutine, const std::string& functionName) {
	_enabledSubroutines[ShaderEnum::FRAGMENT_SHADER][subroutine] = functionName;
}
