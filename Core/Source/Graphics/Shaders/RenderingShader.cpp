#include "CorePch.h"
#include "RenderingShader.h"

#include <iostream>

Nimbus::RenderingShader::RenderingShader() : ShaderProgram() {}

void Nimbus::RenderingShader::applyActiveSubroutines() const {
	if (!_activeSubroutineUniform[ShaderEnum::VERTEX_SHADER].empty())
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, static_cast<uint32_t>(_activeSubroutineUniform[ShaderEnum::VERTEX_SHADER].size()), _activeSubroutineUniform[ShaderEnum::VERTEX_SHADER].data());
	if (!_activeSubroutineUniform[ShaderEnum::FRAGMENT_SHADER].empty())
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, static_cast<uint32_t>(_activeSubroutineUniform[ShaderEnum::FRAGMENT_SHADER].size()), _activeSubroutineUniform[ShaderEnum::FRAGMENT_SHADER].data());
	if (!_activeSubroutineUniform[ShaderEnum::GEOMETRY_SHADER].empty())
		glUniformSubroutinesuiv(GL_GEOMETRY_SHADER, static_cast<uint32_t>(_activeSubroutineUniform[ShaderEnum::GEOMETRY_SHADER].size()), _activeSubroutineUniform[ShaderEnum::GEOMETRY_SHADER].data());
}

uint32_t Nimbus::RenderingShader::createShaderProgram(const char* filename) {
	fmt::println("=========================================");
	if (_handler <= 0) {										// Shader identifier
		_handler = glCreateProgram();
		if (_handler == 0) {
			spdlog::error("Cannot create shader program {}", filename);
			return UINT32_MAX;
		}
	}
	shaderPath = filename;

	// [Vertex shader]
	char fileNameComplete[256];
	strcpy_s(fileNameComplete, filename);
	strcat_s(fileNameComplete, ".vert");

	const uint32_t vertexShaderObject = compileShader(fileNameComplete, GL_VERTEX_SHADER);
	if (vertexShaderObject == 0) {
		return UINT32_MAX;
	}

	// [Fragment shader]
	strcpy_s(fileNameComplete, filename);
	strcat_s(fileNameComplete, ".frag");

	const uint32_t fragmentShaderObject = compileShader(fileNameComplete, GL_FRAGMENT_SHADER);
	if (fragmentShaderObject == 0) {
		return UINT32_MAX;
	}

	// [Geometry shader, optional]
	strcpy_s(fileNameComplete, filename);
	strcat_s(fileNameComplete, ".geom");
	const uint32_t geometryShaderObject = compileShader(fileNameComplete, GL_GEOMETRY_SHADER);

	glAttachShader(_handler, vertexShaderObject);						// Associate shaders with shader program
	glAttachShader(_handler, fragmentShaderObject);
	if (geometryShaderObject != 0)
		glAttachShader(_handler, geometryShaderObject);

	glLinkProgram(_handler);											// Link shader program to check errors
	int32_t linkSuccess = 0;
	int32_t logLen = 0;
	glGetProgramiv(_handler, GL_LINK_STATUS, &linkSuccess);
	glGetProgramiv(_handler, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0) {
		char* cLogString = new char[logLen];
		int32_t written = 0;
		glGetProgramInfoLog(_handler, logLen, &written, cLogString);
		_logString.assign(cLogString);
		delete[] cLogString;
		spdlog::warn("Shader linking log: {}", _logString);
	}

	if (linkSuccess == GL_FALSE) {
		spdlog::error("Could not link shader {}", filename);
		return UINT32_MAX;
	} else {
		spdlog::info("{} linked successfully", filename);
		_linked = true;
	}

	std::cout << std::endl << filename << " shader" << std::endl;
	// Allocate space for subroutines
	int32_t numSubroutines;
	glGetProgramStageiv(_handler, GL_VERTEX_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &numSubroutines);
	_activeSubroutineUniform[ShaderEnum::VERTEX_SHADER].resize(numSubroutines);
	std::ranges::fill(_activeSubroutineUniform[ShaderEnum::VERTEX_SHADER], -1);				// Non valid ID

	glGetProgramStageiv(_handler, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &numSubroutines);
	_activeSubroutineUniform[ShaderEnum::FRAGMENT_SHADER].resize(numSubroutines);
	std::ranges::fill(_activeSubroutineUniform[ShaderEnum::FRAGMENT_SHADER], -1);

	glGetProgramStageiv(_handler, GL_GEOMETRY_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &numSubroutines);
	_activeSubroutineUniform[ShaderEnum::GEOMETRY_SHADER].resize(numSubroutines);
	std::ranges::fill(_activeSubroutineUniform[ShaderEnum::GEOMETRY_SHADER], -1);

	_implementsSubroutine[ShaderEnum::VERTEX_SHADER].resize(ShaderEnum::NUM_VERTEX_SUBROUTINES);
	for (int i = 0; i < ShaderEnum::NUM_VERTEX_SUBROUTINES; ++i) {
		try {
			getSubroutineLocation(ShaderEnum::SubroutineName[ShaderEnum::VERTEX_SHADER][i] + "Uniform", GL_VERTEX_SHADER);
			_implementsSubroutine[ShaderEnum::VERTEX_SHADER][i] = true;
			std::cout << " |-> Supports " << ShaderEnum::SubroutineName[ShaderEnum::VERTEX_SHADER][i] << std::endl;
		} catch (std::runtime_error& e) {
			_implementsSubroutine[ShaderEnum::VERTEX_SHADER][i] = false;
			std::cout << " |-> Doesn't support " << ShaderEnum::SubroutineName[ShaderEnum::VERTEX_SHADER][i] << std::endl;
		}
	}

	_implementsSubroutine[ShaderEnum::GEOMETRY_SHADER].resize(ShaderEnum::NUM_GEOMETRY_SUBROUTINES);
	for (int i = 0; i < ShaderEnum::NUM_GEOMETRY_SUBROUTINES; ++i) {
		try {
			getSubroutineLocation(ShaderEnum::SubroutineName[ShaderEnum::GEOMETRY_SHADER][i] + "Uniform", GL_GEOMETRY_SHADER);
			_implementsSubroutine[ShaderEnum::GEOMETRY_SHADER][i] = true;
			std::cout << " |-> Supports " << ShaderEnum::SubroutineName[ShaderEnum::GEOMETRY_SHADER][i] << std::endl;
		} catch (std::runtime_error& e) {
			_implementsSubroutine[ShaderEnum::GEOMETRY_SHADER][i] = false;
			std::cout << " |-> Doesn't support " << ShaderEnum::SubroutineName[ShaderEnum::GEOMETRY_SHADER][i] << std::endl;
		}
	}

	_implementsSubroutine[ShaderEnum::FRAGMENT_SHADER].resize(ShaderEnum::NUM_FRAGMENT_SUBROUTINES);
	for (int i = 0; i < ShaderEnum::NUM_FRAGMENT_SUBROUTINES; ++i) {
		try {
			getSubroutineLocation(ShaderEnum::SubroutineName[ShaderEnum::FRAGMENT_SHADER][i] + "Uniform", GL_FRAGMENT_SHADER);
			_implementsSubroutine[ShaderEnum::FRAGMENT_SHADER][i] = true;
			std::cout << " |-> Supports " << ShaderEnum::SubroutineName[ShaderEnum::FRAGMENT_SHADER][i] << std::endl;
		} catch (std::runtime_error& e) {
			_implementsSubroutine[ShaderEnum::FRAGMENT_SHADER][i] = false;
			std::cout << " |-> Doesn't support " << ShaderEnum::SubroutineName[ShaderEnum::FRAGMENT_SHADER][i] << std::endl;
		}
	}

	glDeleteShader(vertexShaderObject);
	glDeleteShader(fragmentShaderObject);
	if (geometryShaderObject != 0) glDeleteShader(geometryShaderObject);

	std::cout << "=========================================" << std::endl;
	return _handler;
}

bool Nimbus::RenderingShader::support(const Nimbus::ShaderEnum::ShaderTypes& shaderStage, const uint8_t& subroutine) const {
	return _implementsSubroutine[shaderStage][subroutine];
}

bool Nimbus::RenderingShader::support(const uint8_t& shaderStage, const uint8_t& subroutine) const {
	return _implementsSubroutine[shaderStage][subroutine];
}