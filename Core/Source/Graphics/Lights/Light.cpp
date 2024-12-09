#include "CorePch.h"
#include "Light.h"

#include "LightApplicators.h"
#include "LightAttenuation.h"
#include "LightModel.h"

// [Static members initialization]

const std::unordered_map<std::string, Nimbus::LightModel::LightModelEnum> Nimbus::Light::LIGHT_ENUM_STR =
{
	{ "AMBIENT_LIGHT", LightModel::AMBIENT_LIGHT },
	{ "POINT_LIGHT", LightModel::POINT_LIGHT },
	{ "DIRECTIONAL_LIGHT", LightModel::DIRECTIONAL_LIGHT },
	{ "SPOT_LIGHT", LightModel::SPOT_LIGHT },
	{ "RIM_LIGHT", LightModel::RIM_LIGHT }
};

const std::vector<std::unique_ptr<Nimbus::LightModel>> Nimbus::Light::_lightModel = Nimbus::Light::getLightTypeApplicators();
const std::vector<std::unique_ptr<Nimbus::LightAttenuation>> Nimbus::Light::_lightAttenuation = Nimbus::Light::getLightAttenuationApplicators();

// Public methods

Nimbus::Light::Light()
{
	_name = "New light";
	_lightType = LightModel::POINT_LIGHT;
	_attenuationType = LightAttenuation::DEFAULT_DISTANCE;
	_enabled = true;

	_properties.setIa(vec3(.2f));
	_properties.setId(vec3(2.0f));
	_properties.setIs(vec3(.5f));
	_properties.setPosition(vec3(.0f, 10.0f, 5.0f));
	_properties.setDirection(vec3(.0f, -1.0f, -0.5f));
}

Nimbus::Light::~Light() {}

void Nimbus::Light::applyLight(Nimbus::Material* material, const mat4& viewMatrix, const uint32_t shaderStage01, const uint32_t shaderStage02) {
	Nimbus::Light::_lightModel[_lightType]->applyLight(&_properties, material, viewMatrix, shaderStage01, shaderStage02);
	Nimbus::Light::_lightAttenuation[_attenuationType]->applyAttenuation(&_properties, material);
}

void Nimbus::Light::setAttenuationType(const Nimbus::LightAttenuation::AttenuationModel attenuation) {
	_attenuationType = attenuation;
}

void Nimbus::Light::setLightType(const Nimbus::LightModel::LightModelEnum lightModel) {
	_lightType = lightModel;
}

void Nimbus::Light::toggle() {}

std::vector<std::unique_ptr<Nimbus::LightAttenuation>> Nimbus::Light::getLightAttenuationApplicators() {
	std::vector<std::unique_ptr<Nimbus::LightAttenuation>> applicator(Nimbus::LightAttenuation::PIXAR + 1);

	applicator[Nimbus::LightAttenuation::DEFAULT_DISTANCE].reset(new BasicAttenuation());
	applicator[Nimbus::LightAttenuation::CONTROLLED_DISTANCE].reset(new DistanceAttenuation());
	applicator[Nimbus::LightAttenuation::PIXAR].reset(new PixarAttenuation());

	return applicator;
}

std::vector<std::unique_ptr<Nimbus::LightModel>> Nimbus::Light::getLightTypeApplicators() {
	std::vector<std::unique_ptr<Nimbus::LightModel>> applicator(Nimbus::LightModel::RIM_LIGHT + 1);

	applicator[Nimbus::LightModel::AMBIENT_LIGHT].reset(new AmbientLight());
	applicator[Nimbus::LightModel::DIRECTIONAL_LIGHT].reset(new DirectionalLight());
	applicator[Nimbus::LightModel::POINT_LIGHT].reset(new PointLight());
	applicator[Nimbus::LightModel::RIM_LIGHT].reset(new RimLight());
	applicator[Nimbus::LightModel::SPOT_LIGHT].reset(new SpotLight());

	return applicator;
}

