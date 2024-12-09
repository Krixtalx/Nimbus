#pragma once

namespace Nimbus {
	class Material;

	class LightProperties {
	protected:
		const static vec3		COLOR;

		const static vec3		POSITION, DIRECTION;
		const static float		EXPONENT_S, PENUMBRA_ANGLE, UMBRA_ANGLE;

		const static float		C1, C2, C3;
		const static float		MAX_DISTANCE, MIN_DISTANCE;
		const static float		FACTOR_C, DISTANCE_C, MAX_FACTOR, EXPONENT_SE;

	public:
		// Colors
		vec3		_Ia, _Id, _Is;						//!< Ambient (as well as rim), diffuse and specular colors!

		// Spatial parameters
		vec3		_position;							//!< World position
		vec3		_direction;							//!< Only for directional and spot lights
		float		_exponentS;							//!< Phong exponent
		float		_penumbraAngle;						//!< Boundary angle which delimites the zone completely lighted (attenuation = 1) and the fading area (radians)
		float		_umbraAngle;						//!< Boundary angle to attenuate completely the surface color (radians)

		// Attenuation
		float		_c1, _c2, _c3;						//!< Attenuation factor													(Model 1)
		float		_minDistance, _maxDistance;			//!< Mininum distance -> max distance (attenuation range)				(Model 2)
		float		_factorC, _distC;					//!< Attenuation factor factorC is reached at distC distance			(Model 3: Pixar)
		float		_maxAttFactor;						//!< Maximum attenuation factor											(Model 3: Pixar)
		float		_exponentSE;						//!< Decreases attenuation factor as the distance gets bigger			(Model 3: Pixar)
		float		_k0, _k1;							//!< Derivated attributes from previous variables						(Model 3: Pixar)

		LightProperties();
		virtual ~LightProperties();

		// Getters
		vec3 getIa() const { return _Ia; }
		vec3 getId() const { return _Id; }
		vec3 getIs() const { return _Is; }
		vec3 getPosition() const { return _position; }
		vec3 getDirection() const { return _direction; }
		float getExponentS() const { return _exponentS; }
		float getPenumbraDegrees() const { return _penumbraAngle; }
		float getUmbraDegrees() const { return _umbraAngle; }
		vec3 getAttenuationCoefficients() const { return {_c1, _c2, _c3}; }
		vec2 getDistanceRange() const { return {_minDistance, _maxDistance}; }
		float getFactorC() const { return _factorC; }
		float getDistanceC() const { return _distC; }
		float getMaxAttFactor() const { return _maxAttFactor; }
		float getExponentSE() const { return _exponentSE; }
		vec2 getKCoefficients() const { return {_k0, _k1}; }

		// Setters
		void setIa(const vec3& Ia) { _Ia = Ia; }
		void setId(const vec3& Id) { _Id = Id; }
		void setIs(const vec3& Is) { _Is = Is; }
		void setPosition(const vec3& position);
		void setDirection(const vec3& direction);
		void setExponentS(float exponent);
		void setUmbraPenumbraRange(float innerAngle, float outterAngle);
		void setAttenuationCoefficients(float c1, float c2, float c3);
		void setAttenuationDistanceRange(float minDistance, float maxDistance);
		void setAttenuationCoeffDistance(float attFactor, float distance);
		void setMaxAttenuationFactor(float maxAttFactor);
		void setExponentSE(float exponentSE);

	protected:
		void computePixarAttenuationCoefficients();
	};

	class LightModel {
	public:
		enum LightModelEnum {
			AMBIENT_LIGHT, POINT_LIGHT, DIRECTIONAL_LIGHT, SPOT_LIGHT, RIM_LIGHT
		};

	public:
		virtual void applyLight(LightProperties* lightProperties, Material* material, const mat4& viewMatrix, uint32_t shaderStage01, uint32_t shaderStage02) = 0;
	};
}

