#include "CorePch.h"
#include "ComputeShader.h"


/// [Public methods]

void Nimbus::ComputeShader::applyActiveSubroutines() const{
	this->bind();
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, static_cast<GLsizei>(_activeSubroutineUniform[ShaderEnum::COMPUTE_SHADER].size()), _activeSubroutineUniform[ShaderEnum::COMPUTE_SHADER].data());
}

uint32_t Nimbus::ComputeShader::createShaderProgram(const char* filename) {
	if (_handler <= 0) {										// Shader identifier
		_handler = glCreateProgram();

		if (_handler == 0) {
			spdlog::error("Cannot create shader program: {}!\n", filename);
			return 0;
		}
	}

	char fileNameComplete[256];
	strcpy_s(fileNameComplete, filename);
	strcat_s(fileNameComplete, ".comp");
	const uint32_t vertexShaderObject = compileShader(fileNameComplete, GL_COMPUTE_SHADER);
	if (vertexShaderObject == 0) {
		return 0;
	}

	glAttachShader(_handler, vertexShaderObject);				// Link shader program and compute shader
	glLinkProgram(_handler);									// Link shader program to check errors
	int32_t linkSuccess = 0;
	glGetProgramiv(_handler, GL_LINK_STATUS, &linkSuccess);

	if (linkSuccess == GL_FALSE) {
		int32_t logLen = 0;
		glGetProgramiv(_handler, GL_INFO_LOG_LENGTH, &logLen);

		if (logLen > 0) {
			char* cLogString = new char[logLen];
			int32_t written = 0;

			glGetProgramInfoLog(_handler, logLen, &written, cLogString);
			_logString.assign(cLogString);
			delete[] cLogString;
			spdlog::error("Cannot link shader {}: {}", filename, _logString);
		}

		return 0;
	}
	_linked = true;

	// Reserve space for subroutines
	int32_t numSubroutines;

	glGetProgramStageiv(_handler, GL_COMPUTE_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &numSubroutines);
	_activeSubroutineUniform[ShaderEnum::COMPUTE_SHADER].resize(numSubroutines);
	std::ranges::fill(_activeSubroutineUniform[ShaderEnum::COMPUTE_SHADER], -1);		// Non valid id

	/*std::string compiledShader;
	compiledShader.resize(10000000);
	GLsizei length;
	GLenum binaryFormat;
	glGetProgramBinary(_handler, 10000000, &length, &binaryFormat, compiledShader.data());
	compiledShader.resize(length);
	std::ofstream shaderCompiledFile;
	shaderCompiledFile.open(std::string(fileNameComplete) + "Compiled");
	if (shaderCompiledFile.is_open()) {
		shaderCompiledFile << compiledShader;
	}
	shaderCompiledFile.close();*/

	return _handler;
}

void Nimbus::ComputeShader::execute(const GLuint xNumGroups, const GLuint xWorkGroupSize, const GLuint yNumGroups, const GLuint yWorkGroupSize, const GLuint zNumGroups, const GLuint zWorkGroupSize) const {
	this->bind();
	//validateGroupNumber(xNumGroups, xWorkGroupSize, yNumGroups, yWorkGroupSize, zNumGroups, zWorkGroupSize);
	glDispatchComputeGroupSizeARB(xNumGroups, yNumGroups, zNumGroups, xWorkGroupSize, yWorkGroupSize, zWorkGroupSize);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void Nimbus::ComputeShader::validateGroupNumber(const GLuint xNumGroups, const GLuint yNumGroups, const GLuint zNumGroups,
											 const GLuint xWorkGroupSize, const GLuint yWorkGroupSize, const GLuint zWorkGroupSize) {
	//https://registry.khronos.org/OpenGL/extensions/ARB/ARB_compute_variable_group_size.txt
}

int Nimbus::ComputeShader::getNumGroups(const unsigned arraySize, const WorkGroupAxis axis) {
	return static_cast<int>(std::ceil(static_cast<float>(arraySize) / MAX_WORK_GROUP_SIZE[axis]));
}

int Nimbus::ComputeShader::getWorkGroupSize(const unsigned numGroups, const unsigned arraySize) {
	return static_cast<int>(std::ceil(static_cast<float>(arraySize) / numGroups));
}

void Nimbus::ComputeShader::initializeStaticVariables() {
	glGetIntegeri_v(GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB, 0, &MAX_WORK_GROUP_SIZE[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB, 1, &MAX_WORK_GROUP_SIZE[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB, 2, &MAX_WORK_GROUP_SIZE[2]);

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &MAX_WORK_GROUP_COUNT[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &MAX_WORK_GROUP_COUNT[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &MAX_WORK_GROUP_COUNT[2]);

	glGetInteger64v(GL_MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS_ARB, &MAX_WORK_GROUP_INVOCATIONS);
}