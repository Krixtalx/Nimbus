#pragma once

#include "LightAttenuation.h"
#include "LightModel.h"

namespace Nimbus {
	class Light {
		friend class DetailsWindow;

	protected:
		const static std::unordered_map<std::string, LightModel::LightModelEnum> LIGHT_ENUM_STR;

		const static std::vector<std::unique_ptr<Nimbus::LightModel>>			_lightModel;
		const static std::vector<std::unique_ptr<Nimbus::LightAttenuation>>	_lightAttenuation;

	protected:
		std::string						_name;
		int								_attenuationType;
		bool							_enabled;
		int								_lightType;
		LightProperties					_properties;

	protected:
		static std::vector<std::unique_ptr<Nimbus::LightAttenuation>> getLightAttenuationApplicators();
		static std::vector<std::unique_ptr<Nimbus::LightModel>> getLightTypeApplicators();

	public:
		Light();
		virtual ~Light();

		void applyLight(Nimbus::Material* material, const mat4& viewMatrix, uint32_t shad1, uint32_t shad2);
		LightProperties* getProperties() { return &_properties; }

		void setAttenuationType(Nimbus::LightAttenuation::AttenuationModel attenuation);
		void setLightType(Nimbus::LightModel::LightModelEnum lightModel);
		void toggle();


		std::string getName() {
			return _name;
		};
		void setName(const std::string& name) {
			_name = name;
		}
	};
}

