#pragma once

#include "LightModel.h"

namespace Nimbus {
	class AmbientLight : public LightModel {
		void applyLight(LightProperties* lightProperties, Material* material, const mat4& viewMatrix, uint32_t shaderStage01 = GL_VERTEX_SHADER, uint32_t shaderStage02 = GL_FRAGMENT_SHADER) override;
	};

	class PointLight : public LightModel {
		void applyLight(LightProperties* lightProperties, Material* material, const mat4& viewMatrix, uint32_t shaderStage01 = GL_VERTEX_SHADER, uint32_t shaderStage02 = GL_FRAGMENT_SHADER) override;
	};

	class DirectionalLight : public LightModel {
		void applyLight(LightProperties* lightProperties, Material* material, const mat4& viewMatrix, uint32_t shaderStage01 = GL_VERTEX_SHADER, uint32_t shaderStage02 = GL_FRAGMENT_SHADER) override;
	};

	class SpotLight : public LightModel {
		void applyLight(LightProperties* lightProperties, Material* material, const mat4& viewMatrix, uint32_t shaderStage01 = GL_VERTEX_SHADER, uint32_t shaderStage02 = GL_FRAGMENT_SHADER) override;
	};

	class RimLight : public LightModel {
		void applyLight(LightProperties* lightProperties, Material* material, const mat4& viewMatrix, uint32_t shaderStage01 = GL_VERTEX_SHADER, uint32_t shaderStage02 = GL_FRAGMENT_SHADER) override;
	};
}

