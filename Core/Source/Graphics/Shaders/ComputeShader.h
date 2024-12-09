#pragma once

#include "ShaderProgram.h"

namespace Nimbus {
	class ComputeShader : public ShaderProgram {
	public:
		enum WorkGroupAxis { X_AXIS, Y_AXIS, Z_AXIS };

	protected:
		inline static std::array<GLint, 3> MAX_WORK_GROUP_SIZE = { 0,0,0 };	//!< This value can be useful since the number of groups is not as limited as group size
		inline static std::array<GLint, 3> MAX_WORK_GROUP_COUNT = { 0,0,0 };
		inline static GLint64 MAX_WORK_GROUP_INVOCATIONS = 0;

	public:
		void applyActiveSubroutines() const override;
		GLuint createShaderProgram(const char* filename) override;
		void execute(GLuint xNumGroups, GLuint xWorkGroupSize, GLuint yNumGroups = 1, GLuint yWorkGroupSize = 1, GLuint zNumGroups = 1, GLuint zWorkGroupSize = 1) const;

		// ----------------------------------------
		template<typename T>
		static unsigned getAllowedNumberOfInstances(T data);
		static void validateGroupNumber(const GLuint xNumGroups, const GLuint yNumGroups, const GLuint zNumGroups, const GLuint xWorkGroupSize, const GLuint yWorkGroupSize, const GLuint zWorkGroupSize);
		static GLint getMaxGroupSize(const WorkGroupAxis axis = X_AXIS) { return MAX_WORK_GROUP_SIZE[axis]; }
		static int getNumGroups(unsigned arraySize, WorkGroupAxis axis = X_AXIS);
		static int getWorkGroupSize(unsigned numGroups, unsigned arraySize);
		static void initializeStaticVariables();
	};
}

template<typename T>
unsigned Nimbus::ComputeShader::getAllowedNumberOfInstances(T data) {
	const unsigned size = sizeof(data);
	int32_t limitedMemory;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &limitedMemory);

	return std::floor(limitedMemory / size);
}
