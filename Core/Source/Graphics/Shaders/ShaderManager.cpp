#include "CorePch.h"
#include "ShaderManager.h"

#include "ComputeShader.h"
#include "RenderingShader.h"

// Private methods
Nimbus::ShaderManager::ShaderManager() = default;

Nimbus::ShaderManager::~ShaderManager() = default;

// Public methods

/**
 * Devuelve el rendering shader requerido. En caso de no encontrarse cargado, se carga, compila y enlaza en el momento.
 *
 * \param shaderName nombre del Shader. Se presupone la ruta ./Assets/Shaders/RenderShaders/
 * \return Nimbus::RenderingShader* requerido
 */
Nimbus::RenderingShader* Nimbus::ShaderManager::getRenderingShader(const std::string& shaderName) {
	const auto pair = _renderingShaders.find(shaderName);
	if (pair == _renderingShaders.end()) {
		auto shader = new RenderingShader();
		spdlog::info("Loading shader {}", shaderName);
		shader->createShaderProgram(("Assets/Shaders/RenderShaders/" + shaderName).c_str());
		_renderingShaders.insert(std::make_pair(shaderName, shader));
		return shader;
	}
	return pair->second.get();
}

/**
 * Devuelve el compute shader requerido. En caso de no encontrarse cargado, se carga, compila y enlaza en el momento.
 *
 * \param shaderName nombre del Shader. Se presupone la ruta ./Assets/Shaders/ComputeShaders/
 * \return Nimbus::ComputeShader* requerido
 */
Nimbus::ComputeShader* Nimbus::ShaderManager::getComputeShader(const std::string& shaderName) {
	const auto pair = _computeShaders.find(shaderName);
	if (pair == _computeShaders.end()) {
		auto* shader = new ComputeShader();
		spdlog::info("Loading shader {}", shaderName);
		shader->createShaderProgram(("Assets/Shaders/ComputeShaders/" + shaderName).c_str());
		_computeShaders.insert(std::make_pair(shaderName, shader));
		return shader;
	}
	return pair->second.get();
}

void Nimbus::ShaderManager::clear() {
	_renderingShaders.clear();
	_computeShaders.clear();
}

void Nimbus::ShaderManager::clearComputeShaders() {
	_computeShaders.clear();
}

void Nimbus::ShaderManager::clearRenderShaders() {
	_renderingShaders.clear();
}
