#pragma once
#include "ShaderEnums.h"

namespace Nimbus {
	class ShaderProgram {
	protected:
		const static std::string MODULE_HEADER;
		const static std::string MODULE_FILE_CHAR_1;
		const static std::string MODULE_FILE_CHAR_2;
		static std::unordered_map<std::string, std::string> _moduleCode;			//!< Modules that are already loaded
		static uint32_t _activeShaderProgram;

		std::string shaderPath;
		std::vector<uint32_t> _activeSubroutineUniform[ShaderEnum::COMPUTE_SHADER + 1];			//!< Active uniform for each subroutine for each shader type
		uint32_t				_handler;												//!< Shader program id in GPU
		bool				_linked;												//!< Flag which tell us if the shader has been linked correctly
		std::string			_logString;												//!< Error message got from the last operation with the shader
		std::unordered_map<std::string, int32_t> uniformLocation;
		std::unordered_map<std::string, int32_t> subroutineLocation;


		virtual uint32_t compileShader(const char* filename, GLenum shaderType);
		bool fileExists(const std::string& fileName);
		bool includeLibraries(std::string& shaderContent);
		virtual bool loadFileContent(const std::string& filename, std::string& content);
		bool showErrorMessage(const std::string& variableName);
		int32_t getUniformLocation(const std::string& name);
		int32_t getSubroutineLocation(const std::string& subroutine, const GLenum& shaderStage);
	public:
		ShaderProgram();
		virtual ~ShaderProgram();

		virtual void applyActiveSubroutines() const = 0;
		virtual uint32_t createShaderProgram(const char* filename) = 0;

		void setSubroutineUniform(const uint8_t& shaderType, const uint8_t& subroutine, const std::string& functionName);
		void setUniform(const std::string& name, const GLfloat& value);
		void setUniform(const std::string& name, const int32_t& value);
		void setUniform(const std::string& name, const uint32_t& value);
		void setUniform(const std::string& name, const mat4& value);
		void setUniform(const std::string& name, const vec2& value);
		void setUniform(const std::string& name, const uvec2& value);
		void setUniform(const std::string& name, const ivec2& value);
		void setUniform(const std::string& name, const vec3& value);
		void setUniform(const std::string& name, const vec4& value);
		void setUniform(const std::string& name, const vec4& value, unsigned count);
		void setUniform(const std::string& name, const dvec4& value, unsigned count);

		void bind() const;
	};
}

