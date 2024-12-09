#pragma once
#define RESTART_PRIMITIVE_INDEX 0xFFFFFFFF
#include "glm/packing.hpp"

#include "Shaders/ShaderEnums.h"

namespace Nimbus {
	class VAO final {
	public:
		//Problematico cuando se sobrepasen 16 vbos
		enum VBO_slots : uint8_t {
			VBO_POSITION,
			VBO_NORMAL,
			VBO_TEXT_COORD,
			VBO_TANGENT,
			VBO_COLOR_RGB,
			VBO_THERMAL,
			VBO_MULTISPECTRAL,
			VBO_HYPERSPECTRAL,
			VBO_ND_VEGETATION_INDEX,
			VBO_R_VEGETATION_INDEX,
			VBO_GR_VEGETATION_INDEX,
			VBO_NDRE_VEGETATION_INDEX,
			VBO_LIDAR_INTENSITY,
			VBO_SCAN_ANGLE,
			NUM_VBOS
		};

		enum IBO_slots {
			IBO_TRIANGLE,
			NUM_IBOS
		};

		struct VBOSlot_definition {
			uint32_t		_atomicElementType;
			uint32_t		_numBytes;
		};

		struct Vertex {
			vec3		_position, _normal;
			vec2		_textCoord;
		};

		struct Point {
			vec3		_point;
			unsigned	_rgb;

			/**
			*	@return RGB color packed as a single unsigned value.
			*/
			static unsigned get8BitRGBColor(const vec3& rgb) { return packUnorm4x8(vec4(rgb, 1.0f) / 255.0f); }

			static unsigned getFloatRGBColor(const vec3& rgb) { return packUnorm4x8(vec4(rgb, 1.0f)); }

			static vec4 getRGBVec4(const unsigned& uintRGB) { return glm::unpackUnorm4x8(uintRGB); }

			/**
			*	@return RGB color packed as a single unsigned value.
			*/
			vec3 getRGBVec3() const { return getRGBVec4(_rgb); }

			/**
			*	@brief Packs an RGB color into an unsigned value.
			*/
			void saveRGB(const vec3& rgb) { _rgb = get8BitRGBColor(rgb); }

		};

	private:
		uint32_t				_vao;
		std::vector<uint32_t> _vbos;
		std::vector<uint32_t> _ibos;
		bool _defineVBO[NUM_VBOS]{};

		void defineInterleavedVBOForTriangleMesh();
		void defineInterleavedVBOForPointCloud();
		void defineNonInterleaveVBO(uint32_t vboId, size_t structSize, uint32_t elementType, VBO_slots slot);

	public:
		VAO(bool pointCloud = true);
		virtual ~VAO();

		void drawObject(IBO_slots ibo, uint32_t openGLPrimitive, uint32_t numIndices) const;
		void drawObject(uint32_t openGLPrimitive, uint32_t numIndices) const;

		template<typename T>
		void setVBOData(VBO_slots vbo, T* geometryData, uint32_t size, uint32_t changeFrequency = GL_STATIC_DRAW);
		void setVBOData(const std::vector<Vertex>& vertices, uint32_t changeFrequency = GL_STATIC_DRAW) const;
		void setVBOData(const std::vector<Point>& points, uint32_t changeFrequency = GL_STATIC_DRAW) const;
		void setIBOData(IBO_slots ibo, const std::vector<uint32_t>& indices, uint32_t changeFrequency = GL_STATIC_DRAW) const;
		void clearVBOData(VBO_slots vbo);
		bool support(const ShaderEnum::VertexSubroutines& subroutine) const;
		bool support(const ShaderEnum::GeometrySubroutines& subroutine) const;
		bool support(const ShaderEnum::FragmentSubroutines& subroutine) const;
		bool support(const ShaderEnum::ShaderTypes& shaderType, const uint8_t& subroutine) const;
	};

	template<typename T>
	void VAO::setVBOData(const VBO_slots vbo, T* geometryData, const uint32_t size, const uint32_t changeFrequency) {
		if (vbo != NUM_VBOS) {
			glBindVertexArray(_vao);
			if (_vbos[vbo] == UINT32_MAX) {
				glCreateBuffers(1, &_vbos[vbo]);
			}
			glNamedBufferData(_vbos[vbo], size * sizeof(T), geometryData, changeFrequency);
			defineNonInterleaveVBO(_vbos[vbo], sizeof(T), GL_FLOAT, vbo);
		}
	}
}

