#pragma once

#include "ShaderProgram.h"

namespace Nimbus {
	class RenderingShader : public ShaderProgram {
		std::vector<bool> _implementsSubroutine[ShaderEnum::ShaderTypes::NUM_SHADER_TYPES];
	public:
		RenderingShader();
		void applyActiveSubroutines() const override;
		uint32_t createShaderProgram(const char* filename) override;
		bool support(const ShaderEnum::ShaderTypes& shaderStage, const uint8_t& subroutine) const;
		bool support(const uint8_t& shaderStage, const uint8_t& subroutine) const;
	};
}

