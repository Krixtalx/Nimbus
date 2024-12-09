#include "CorePch.h"
#include "LightModel.h"

const vec3	Nimbus::LightProperties::COLOR = vec3(0.4f, 0.4f, 0.4f);

const vec3	Nimbus::LightProperties::POSITION = vec3(0.0f, 20.0f, 0.0f);
const vec3	Nimbus::LightProperties::DIRECTION = vec3(0.0f, -1.0f, 0.0f);
const float Nimbus::LightProperties::EXPONENT_S = 3.0f;
const float Nimbus::LightProperties::PENUMBRA_ANGLE = 20.0f;
const float Nimbus::LightProperties::UMBRA_ANGLE = 35.0f;

const float Nimbus::LightProperties::C1 = 1.0f;
const float Nimbus::LightProperties::C2 = 0.0f;
const float Nimbus::LightProperties::C3 = 0.0f;
const float Nimbus::LightProperties::MAX_DISTANCE = 20.0f;
const float Nimbus::LightProperties::MIN_DISTANCE = 4.0f;
const float Nimbus::LightProperties::FACTOR_C = 0.6f;
const float Nimbus::LightProperties::DISTANCE_C = 10.0f;
const float Nimbus::LightProperties::MAX_FACTOR = 0.3f;
const float Nimbus::LightProperties::EXPONENT_SE = 3.0f;

Nimbus::LightProperties::LightProperties() {
	_Ia = _Id = _Is = COLOR;
	_position = POSITION;
	_direction = DIRECTION;
	_exponentS = EXPONENT_S;
	_penumbraAngle = PENUMBRA_ANGLE; _umbraAngle = UMBRA_ANGLE;

	_c1 = C1; _c2 = C2; _c3 = C3;
	_maxDistance = MAX_DISTANCE; _minDistance = MIN_DISTANCE;
	_factorC = FACTOR_C;
	_distC = DISTANCE_C;
	_maxAttFactor = MAX_FACTOR;
	_exponentSE = EXPONENT_SE;

	this->computePixarAttenuationCoefficients();
}

Nimbus::LightProperties::~LightProperties() {}

void Nimbus::LightProperties::setPosition(const vec3& position) {
	_position = position;
}

void Nimbus::LightProperties::setDirection(const vec3& direction) {
	_direction = glm::normalize(direction);
}

void Nimbus::LightProperties::setExponentS(const float exponent) {
	_exponentS = exponent;
}

void Nimbus::LightProperties::setUmbraPenumbraRange(const float innerAngle, const float outterAngle) {
	_penumbraAngle = innerAngle;
	_umbraAngle = outterAngle;
}

void Nimbus::LightProperties::setAttenuationCoefficients(const float c1, const float c2, const float c3) {
	_c1 = c1; _c2 = c2; _c3 = c3;
}

void Nimbus::LightProperties::setAttenuationDistanceRange(const float minDistance, const float maxDistance) {
	_minDistance = minDistance;
	_maxDistance = glm::clamp(maxDistance, minDistance, FLT_MAX);
}

void Nimbus::LightProperties::setAttenuationCoeffDistance(const float attFactor, const float distance) {
	_factorC = attFactor; _distC = distance;
	this->computePixarAttenuationCoefficients();
}

void Nimbus::LightProperties::setMaxAttenuationFactor(const float maxAttFactor) {
	_maxAttFactor = maxAttFactor;
	this->computePixarAttenuationCoefficients();
}

void Nimbus::LightProperties::setExponentSE(const float exponentSE) {
	_exponentSE = exponentSE;
	this->computePixarAttenuationCoefficients();
}

void Nimbus::LightProperties::computePixarAttenuationCoefficients() {
	_k0 = std::log(_factorC / _maxAttFactor) / std::log(glm::e<float>());
	_k1 = _exponentSE / _k0;
}
