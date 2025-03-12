#pragma once
#include "glad/glad.h"															

namespace Nimbus {
	struct ShaderEnum {
		enum ShaderTypes : uint8_t {
			VERTEX_SHADER,
			GEOMETRY_SHADER,
			FRAGMENT_SHADER,
			COMPUTE_SHADER,
			NUM_SHADER_TYPES
		};

		enum VertexSubroutines : uint8_t {
			VERTEX_COLOR_SOURCE,
			FILTER_VEGETATION_INDEX,
			NUM_VERTEX_SUBROUTINES
		};

		enum GeometrySubroutines : uint8_t {
			DRAW_NORMALS,
			NUM_GEOMETRY_SUBROUTINES
		};

		enum FragmentSubroutines : uint8_t {
			LIGHTING,
			LIGHT_ATTENUATION,
			FRAGMENT_COLOR_SOURCE,
			NORMAL_MAPPING,
			NUM_FRAGMENT_SUBROUTINES
		};

		enum ComputeSubroutines : uint8_t {
			GET_COLOR,
			EDL,
			NUM_COMPUTE_SUBROUTINES
		};

		inline static std::vector<std::string> ShaderName
		{
			"VertexShader", "GeometryShader", "FragmentShader", "ComputeShader"
		};

		inline static std::vector<std::string> SubroutineName[ShaderTypes::NUM_SHADER_TYPES] //[ShaderType][Subroutine]
		{
			//Vertex Shader
			{"VertexColorSource",
			"FilterBasedOnValue"},

			//Geometry Shader
			{"DrawNormals"},

			//Fragment Shader
			{"LightType",
			"AttenuationType",
			"FragmentColorSource",
			"NormalMapping"},

			//Compute Shader
			{"GetColor",
			"GetShadingFactor",
			   "GetAttribute" }
		};

		inline static std::vector<std::vector<std::string>> SubroutineUniformNames[ShaderTypes::NUM_SHADER_TYPES] //[ShaderType][Subroutine][SubroutineUniform]
		{
			//== Vertex Shader ==//
			{
				{"usePointColor", "useFusionDataAsColor", "useNormalAsColor", "useClassColor", "useMeshletColor", "useLodNumberColor"}, //VertexColorSource
				{"dontUseFilter", "vegetationFilter"} //FilterBasedOnValue
			},

			//== Geometry Shader ==//
			{
				{"dontDrawNormals", "drawNormals"} //DrawNormals
			},

			//== Fragment Shader ==//
			{
				{"ambientLight", "pointLight", "directionalLight", "spotLight", "rimLight"}, //LightType
				{"basicAttenuation", "rangedAttenuation", "pixarAttenuation"}, //AttenuationType
				{"useMaterialColor", "useTextureColor" }, //FragmentColorSource
				{"useNormalFromModel", "useNormalFromNormalMap"} //NormalMapping
			},

			//== Compute Shader ==//
			{
				{"getColorUnpackingUint", "getColorFromFloat"} //GetColor
			}
		};

		static ShaderTypes fromOpenGLToShaderTypes(const GLenum& shaderType) {
			switch (shaderType) {
			case GL_VERTEX_SHADER: return ShaderTypes::VERTEX_SHADER;
			case GL_FRAGMENT_SHADER: return ShaderTypes::FRAGMENT_SHADER;
			case GL_GEOMETRY_SHADER: return ShaderTypes::GEOMETRY_SHADER;
			case GL_COMPUTE_SHADER: return ShaderTypes::COMPUTE_SHADER;
			default: return ShaderTypes::VERTEX_SHADER;
			}
		}

		static GLenum fromShaderTypesToOpenGL(const ShaderTypes& shaderType) {
			switch (shaderType) {
			case ShaderTypes::VERTEX_SHADER: return GL_VERTEX_SHADER;
			case ShaderTypes::FRAGMENT_SHADER: return  GL_FRAGMENT_SHADER;
			case ShaderTypes::GEOMETRY_SHADER: return GL_GEOMETRY_SHADER;
			case ShaderTypes::COMPUTE_SHADER: return GL_COMPUTE_SHADER;
			default: return GL_VERTEX_SHADER;
			}
		}

		static GLenum fromShaderTypesToOpenGL(const uint8_t& shaderType) {
			switch (shaderType) {
			case ShaderTypes::VERTEX_SHADER: return GL_VERTEX_SHADER;
			case ShaderTypes::FRAGMENT_SHADER: return  GL_FRAGMENT_SHADER;
			case ShaderTypes::GEOMETRY_SHADER: return GL_GEOMETRY_SHADER;
			case ShaderTypes::COMPUTE_SHADER: return GL_COMPUTE_SHADER;
			default: return GL_VERTEX_SHADER;
			}
		}
	};
}