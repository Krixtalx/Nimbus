#pragma once

#include <map>
#include "Utilities/Singleton.h"

namespace Nimbus {
	class RenderingShader;
	class ComputeShader;

	class ShaderManager : public Singleton<ShaderManager> {
		friend class Singleton;

	private:
		std::map<std::string, std::unique_ptr<Nimbus::RenderingShader>>	_renderingShaders;
		std::map<std::string, std::unique_ptr<Nimbus::ComputeShader>>		_computeShaders;

	private:
		ShaderManager();

	public:
		virtual ~ShaderManager();
		RenderingShader* getRenderingShader(const std::string& shaderName);
		ComputeShader* getComputeShader(const std::string& shaderName);
		void clear();
		void clearComputeShaders();
		void clearRenderShaders();
	};
}

