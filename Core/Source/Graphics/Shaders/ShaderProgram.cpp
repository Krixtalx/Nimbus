#include "CorePch.h"
#include "ShaderProgram.h"

// Static variables initialization

const std::string Nimbus::ShaderProgram::MODULE_HEADER = "#include";
const std::string Nimbus::ShaderProgram::MODULE_FILE_CHAR_1 = "<";
const std::string Nimbus::ShaderProgram::MODULE_FILE_CHAR_2 = ">";
std::unordered_map<std::string, std::string> Nimbus::ShaderProgram::_moduleCode;
uint32_t Nimbus::ShaderProgram::_activeShaderProgram = UINT32_MAX;

// Public methods

Nimbus::ShaderProgram::ShaderProgram()
	: _handler(0), _linked(false) {
}

Nimbus::ShaderProgram::~ShaderProgram() {
	uniformLocation.clear();
	subroutineLocation.clear();
	glDeleteProgram(_handler);
}

/**
 * \brief Select the subroutine function to enable.
 * \param shaderType VERTEX_SHADER, GEOMETRY_SHADER or FRAGMENT_SHADER
 * \param subroutine subroutine ID (see ShaderEnums)
 * \param functionName name of the function of the subroutine that is going to be used
 * \throw std::runtime_error if the subroutine or subroutine function specified isn't found in the shader.
 */
void Nimbus::ShaderProgram::setSubroutineUniform(const uint8_t& shaderType, const uint8_t& subroutine, const std::string& functionName) {
	const GLenum glShaderType = ShaderEnum::fromShaderTypesToOpenGL(shaderType);
	const int32_t subroutineID = getSubroutineLocation(ShaderEnum::SubroutineName[shaderType][subroutine] + "Uniform",
		glShaderType);
	const int32_t uniformID = glGetSubroutineIndex(_handler, glShaderType, functionName.c_str());

	if (uniformID >= 0) {			// OpenGL returns -1 if subroutine or uniform doesn't exist
		_activeSubroutineUniform[shaderType][subroutineID] = uniformID;
	} else {
		throw std::runtime_error("[ShaderProgram::setSubroutineUniform]: subroutine function " + functionName + " not found in shader");
	}
}


void Nimbus::ShaderProgram::setUniform(const std::string& name, const GLfloat& value) {
	const int32_t location = getUniformLocation(name);
	glProgramUniform1f(_handler, location, value);
}

void Nimbus::ShaderProgram::setUniform(const std::string& name, const int32_t& value) {
	const int32_t location = getUniformLocation(name);
	glProgramUniform1i(_handler, location, value);
}

void Nimbus::ShaderProgram::setUniform(const std::string& name, const uint32_t& value) {
	const int32_t location = getUniformLocation(name);
	glProgramUniform1ui(_handler, location, value);
}

void Nimbus::ShaderProgram::setUniform(const std::string& name, const mat4& value) {
	const int32_t location = getUniformLocation(name);
	glProgramUniformMatrix4fv(_handler, location, 1, GL_FALSE, &value[0][0]);
}

void Nimbus::ShaderProgram::setUniform(const std::string& name, const vec2& value) {
	const int32_t location = getUniformLocation(name);
	glProgramUniform2fv(_handler, location, 1, &value[0]);
}

void Nimbus::ShaderProgram::setUniform(const std::string& name, const uvec2& value) {
	const int32_t location = getUniformLocation(name);
	glProgramUniform2uiv(_handler, location, 1, &value[0]);
}

void Nimbus::ShaderProgram::setUniform(const std::string& name, const ivec2& value) {
	const int32_t location = getUniformLocation(name);
	glProgramUniform2iv(_handler, location, 1, &value[0]);
}

void Nimbus::ShaderProgram::setUniform(const std::string& name, const vec3& value) {
	const int32_t location = getUniformLocation(name);
	glProgramUniform3fv(_handler, location, 1, &value[0]);
}

void Nimbus::ShaderProgram::setUniform(const std::string& name, const vec4& value) {
	const int32_t location = getUniformLocation(name);
	glProgramUniform4fv(_handler, location, 1, &value[0]);
}

void Nimbus::ShaderProgram::setUniform(const std::string& name, const vec4& value, const unsigned count) {
	const int32_t location = getUniformLocation(name);
	glProgramUniform4fv(_handler, location, count, &value[0]);
}

void Nimbus::ShaderProgram::setUniform(const std::string& name, const dvec4& value, const unsigned count) {
	const int32_t location = getUniformLocation(name);
	glProgramUniform4dv(_handler, location, count, &value[0]);
}

void Nimbus::ShaderProgram::bind() const {
	if ((_handler > 0) && (_linked)) {			// Is the program created and linked?
		if (_activeShaderProgram != _handler) {
			glUseProgram(_handler);
			_activeShaderProgram = _handler;
		}
	} else {
		spdlog::error("Cannot bind shader program {} because is not created or linked", shaderPath);
	}
}

/// [Protected methods]

uint32_t Nimbus::ShaderProgram::compileShader(const char* filename, const GLenum shaderType) {
	if (!fileExists(filename)) {
		if (shaderType != GL_GEOMETRY_SHADER)
			spdlog::critical("Shader source file {} not found", filename);

		return 0;
	}

	std::string shaderSourceString;
	if (!loadFileContent(std::string(filename), shaderSourceString)) {						// Read shader code
		spdlog::critical("Could not open shader {} source file", filename);
		return 0;
	}

	if (!includeLibraries(shaderSourceString)) {										// Libraries code not found 
		spdlog::error("Could not include the specified modules");
		return 0;
	}

	const uint32_t shaderHandler = glCreateShader(shaderType);
	if (shaderHandler == 0) {
		spdlog::critical("Could not create shader objects");
		return 0;
	}

	const char* shaderSourceCString = shaderSourceString.c_str();							// Compile shader code
	glShaderSource(shaderHandler, 1, &shaderSourceCString, nullptr);
	glCompileShader(shaderHandler);

	int32_t compileResult = 0;
	int32_t logLen = 0;
	_logString = "";
	glGetShaderiv(shaderHandler, GL_COMPILE_STATUS, &compileResult);					// Result
	glGetShaderiv(shaderHandler, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0) {
		const auto cLogString = new char[logLen];
		int32_t written = 0;

		glGetShaderInfoLog(shaderHandler, logLen, &written, cLogString);
		_logString.assign(cLogString);

		delete[] cLogString;
		spdlog::warn("Shader log: {}", _logString);
	}
	spdlog::info("{} {}", shaderType, (compileResult == GL_FALSE ? " compiled failed" : " compiled sucessfully"));

	return shaderHandler;
}

bool Nimbus::ShaderProgram::fileExists(const std::string& fileName) {
	struct stat info;

	const int ret = stat(fileName.c_str(), &info);

	return 0 == ret;
}

bool Nimbus::ShaderProgram::includeLibraries(std::string& shaderContent) {
	size_t pos = shaderContent.find(MODULE_HEADER);

	while (pos != std::string::npos) {
		const std::size_t char_1 = shaderContent.find(MODULE_FILE_CHAR_1, pos);
		const std::size_t char_2 = shaderContent.find(MODULE_FILE_CHAR_2, pos);

		if ((char_1 == std::string::npos) || (char_2 == std::string::npos)) {				// Incorrect syntax
			return false;
		}

		const std::string module = shaderContent.substr(char_1 + 1, char_2 - char_1 - 1);
		if (!fileExists(module)) {														// Library refers to a new file, does it exist?
			return false;
		}

		const auto moduleCode = _moduleCode.find(module);								// If file is already read we can just retrieve the string
		std::string moduleCodeStr;
		if (moduleCode == _moduleCode.end()) {
			if (!loadFileContent(module, moduleCodeStr)) {
				return false;
			}

			_moduleCode[module] = moduleCodeStr;
		} else {
			moduleCodeStr = moduleCode->second;
		}

		shaderContent.erase(shaderContent.begin() + pos, shaderContent.begin() + char_2 + 1);			// Replace string in shader code
		shaderContent.insert(pos, moduleCodeStr);
		pos = shaderContent.find(MODULE_HEADER);
	}

	return true;
}

bool Nimbus::ShaderProgram::loadFileContent(const std::string& filename, std::string& content) {
	std::ifstream shaderSourceFile;
	shaderSourceFile.open(filename);

	if (!shaderSourceFile) {
		return false;
	}

	std::stringstream shaderSourceStream;
	shaderSourceStream << shaderSourceFile.rdbuf();
	content = shaderSourceStream.str();
	shaderSourceFile.close();

	return true;
}

bool Nimbus::ShaderProgram::showErrorMessage(const std::string& variableName) {
	spdlog::error("Could not find shader slot for {}", variableName);
	return false;
}


/**
 * \brief Look for uniform location and stores it in uniformLocation map.
 * \throw std::runtime_error in case that specified uniform name doesn't exist in shader.
 * \param name Uniform name
 * \return uniform id
 */
int32_t Nimbus::ShaderProgram::getUniformLocation(const std::string& name) {
	int32_t location;
	const auto pos = uniformLocation.find(name);
	if (pos == uniformLocation.end()) {
		location = glGetUniformLocation(_handler, name.c_str());

		if (location >= 0) {
			uniformLocation.insert(std::make_pair(name, location));
		} else {
			throw std::runtime_error("[ShaderProgram]: No se ha encontrado ningun uniform con el nombre " + name);
		}
	} else {
		location = pos->second;
	}
	return location;
}

/**
 * \brief Look for subroutine location and stores it in subroutineLocation map.
 * \throw std::runtime_error in case that specified subroutine name doesn't exist in shader
 * \param subroutine subroutine name
 * \param shaderStage GL_VERTEX_SHADER, GL_GEOMETRY_SHADER or GL_FRAGMENT_SHADER
 * \return subroutine location id
 */
int32_t Nimbus::ShaderProgram::getSubroutineLocation(const std::string& subroutine, const GLenum& shaderStage) {
	int32_t location;
	const auto pos = subroutineLocation.find(subroutine);
	if (pos == subroutineLocation.end()) {
		location = glGetSubroutineUniformLocation(_handler, shaderStage, subroutine.c_str());

		if (location >= 0) {
			subroutineLocation.insert(std::make_pair(subroutine, location));
		} else {
			throw std::runtime_error("[ShaderProgram]: No se ha encontrado ninguna subrutina con el nombre " + subroutine);
		}
	} else {
		location = pos->second;
	}
	return location;
}

