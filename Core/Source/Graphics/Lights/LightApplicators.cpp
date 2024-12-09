#include "CorePch.h"
#include "LightApplicators.h"

#include "Graphics/Material.h"
#include "Graphics/Shaders/RenderingShader.h"
#include "Graphics/Shaders/ShaderEnums.h"

void Nimbus::AmbientLight::applyLight(LightProperties* lightProperties, Material* material, const mat4& viewMatrix, const uint32_t shaderStage01, const uint32_t shaderStage02) {
	const auto shader = material->getAssociatedShader();
	material->setSubroutine(ShaderEnum::LIGHTING, "ambientLight");
	shader->setUniform("Ia", lightProperties->getIa());
}

void Nimbus::PointLight::applyLight(LightProperties* lightProperties, Material* material, const mat4& viewMatrix, const uint32_t shaderStage01, const uint32_t shaderStage02) {
	const auto shader = material->getAssociatedShader();
	material->setSubroutine(ShaderEnum::LIGHTING, "pointLight");

	shader->setUniform("lightPosition", vec3(viewMatrix * vec4(lightProperties->getPosition(), 1.0f)));
	shader->setUniform("Ia", lightProperties->getIa());
	shader->setUniform("Id", lightProperties->getId());
	shader->setUniform("Is", lightProperties->getIs());
}

void Nimbus::DirectionalLight::applyLight(LightProperties* lightProperties, Material* material, const mat4& viewMatrix, const uint32_t shaderStage01, const uint32_t shaderStage02) {
	const auto shader = material->getAssociatedShader();
	material->setSubroutine(ShaderEnum::LIGHTING, "directionalLight");

	shader->setUniform("lightDirection", vec3(viewMatrix * vec4(lightProperties->getDirection(), 0.0f)));
	shader->setUniform("Id", lightProperties->getId());
	shader->setUniform("Is", lightProperties->getIs());
}

void Nimbus::SpotLight::applyLight(LightProperties* lightProperties, Material* material, const mat4& viewMatrix, const uint32_t shaderStage01, const uint32_t shaderStage02) {
	const auto shader = material->getAssociatedShader();
	material->setSubroutine(ShaderEnum::LIGHTING, "spotLight");

	const float cosUmbraRad = std::cos(glm::radians(lightProperties->getUmbraDegrees()));
	const float cosPenumbraRad = std::cos(glm::radians(lightProperties->getPenumbraDegrees()));

	shader->setUniform("lightPosition", vec3(viewMatrix * vec4(lightProperties->getPosition(), 1.0f)));
	shader->setUniform("lightDirection", vec3(viewMatrix * vec4(lightProperties->getDirection(), 0.0f)));
	shader->setUniform("cosUmbra", cosUmbraRad);
	shader->setUniform("cosPenumbra", cosPenumbraRad);
	shader->setUniform("exponentS", lightProperties->getExponentS());
	shader->setUniform("Id", lightProperties->getId());
	shader->setUniform("Is", lightProperties->getIs());
}

void Nimbus::RimLight::applyLight(LightProperties* lightProperties, Material* material, const mat4& viewMatrix, const uint32_t shaderStage01, const uint32_t shaderStage02) {
	const auto shader = material->getAssociatedShader();
	material->setSubroutine(ShaderEnum::LIGHTING, "rimLight");

	shader->setUniform("Ia", lightProperties->getIa());
}
