#include "CorePch.h"
#include "LightAttenuation.h"

#include "Graphics/Material.h"
#include "Graphics/Shaders/RenderingShader.h"
#include "Graphics/Shaders/ShaderEnums.h"

void Nimbus::BasicAttenuation::applyAttenuation(LightProperties* lightProperties, Nimbus::Material* material) {
	vec3 coefficients = lightProperties->getAttenuationCoefficients();
	const auto shader = material->getAssociatedShader();
	//shader->setSubroutineUniform(GL_FRAGMENT_SHADER, "AttenuationTypeUniform", "basicAttenuation");
	material->setSubroutine(ShaderEnum::LIGHT_ATTENUATION, "basicAttenuation");
	shader->setUniform("c1", coefficients[0]);
	shader->setUniform("c2", coefficients[1]);
	shader->setUniform("c3", coefficients[2]);
}

void Nimbus::DistanceAttenuation::applyAttenuation(LightProperties* lightProperties, Nimbus::Material* material) {
	vec2 distances = lightProperties->getDistanceRange();

	const auto shader = material->getAssociatedShader();
	//shader->setSubroutineUniform(GL_FRAGMENT_SHADER, "AttenuationTypeUniform", "rangedAttenuation");
	material->setSubroutine(ShaderEnum::LIGHT_ATTENUATION, "rangedAttenuation");
	shader->setUniform("minDistance", distances[0]);
	shader->setUniform("maxDistance", distances[1]);
}

void Nimbus::PixarAttenuation::applyAttenuation(LightProperties* lightProperties, Nimbus::Material* material) {
	vec2 coefficients = lightProperties->getKCoefficients();

	const auto shader = material->getAssociatedShader();
	//shader->setSubroutineUniform(GL_FRAGMENT_SHADER, "AttenuationTypeUniform", "pixarAttenuation");
	material->setSubroutine(ShaderEnum::LIGHT_ATTENUATION, "pixarAttenuation");
	shader->setUniform("fMax", lightProperties->getMaxAttFactor());
	shader->setUniform("fC", lightProperties->getFactorC());
	shader->setUniform("distC", lightProperties->getDistanceC());
	shader->setUniform("exponentSE", lightProperties->getExponentSE());
	shader->setUniform("k0", coefficients[0]);
	shader->setUniform("k1", coefficients[1]);
}
