#include "CorePch.h"
#include "VAO.h"

// Public methods

Nimbus::VAO::VAO(const bool pointCloud) : _vao(UINT32_MAX) {
	glCreateVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	for (auto& define : _defineVBO) {
		define = false;
	}

	// VBOs
	_vbos.resize(NUM_VBOS);
	for (auto& vbo : _vbos) {
		vbo = UINT32_MAX;
	}

	if (pointCloud) {
		this->defineInterleavedVBOForPointCloud();
	} else {
		this->defineInterleavedVBOForTriangleMesh();
	}

	// IBOs
	_ibos.resize(NUM_IBOS);
	glCreateBuffers(static_cast<GLsizei>(_ibos.size()), _ibos.data());
}

Nimbus::VAO::~VAO() {
	glDeleteBuffers(static_cast<GLsizei>(_vbos.size()), _vbos.data());
	glDeleteBuffers(static_cast<GLsizei>(_ibos.size()), _ibos.data());
	glDeleteVertexArrays(1, &_vao);
}

void Nimbus::VAO::drawObject(const IBO_slots ibo, const uint32_t openGLPrimitive, const uint32_t numIndices) const {
	glBindVertexArray(_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibos[ibo]);
	glDrawElements(openGLPrimitive, numIndices, GL_UNSIGNED_INT, nullptr);
}

void Nimbus::VAO::drawObject(const uint32_t openGLPrimitive, const uint32_t numIndices) const {
	glBindVertexArray(_vao);
	glDrawElements(openGLPrimitive, numIndices, GL_UNSIGNED_INT, nullptr);
}

void Nimbus::VAO::setVBOData(const std::vector<Vertex>& vertices, const uint32_t changeFrequency) const {
	glBindVertexArray(_vao);
	glNamedBufferData(_vbos[0], vertices.size() * sizeof(VAO::Vertex), vertices.data(), changeFrequency);
}

void Nimbus::VAO::setVBOData(const std::vector<Point>& points, const uint32_t changeFrequency) const {
	glBindVertexArray(_vao);
	glNamedBufferData(_vbos[0], points.size() * sizeof(VAO::Point), points.data(), changeFrequency);
}

void Nimbus::VAO::setIBOData(const IBO_slots ibo, const std::vector<uint32_t>& indices, const uint32_t changeFrequency) const {
	glBindVertexArray(_vao);
	glNamedBufferData(_ibos[ibo], indices.size() * sizeof(uint32_t), indices.data(), changeFrequency);
	glVertexArrayElementBuffer(_vao, _ibos[ibo]);
}

void Nimbus::VAO::clearVBOData(const VBO_slots vbo) {
	if (vbo != NUM_VBOS) {
		glDeleteBuffers(1, &_vbos[vbo]);
		_vbos[vbo] = UINT32_MAX;
	}
}

bool Nimbus::VAO::support(const ShaderEnum::VertexSubroutines& subroutine) const {
	return support(ShaderEnum::VERTEX_SHADER, subroutine);
}

bool Nimbus::VAO::support(const ShaderEnum::GeometrySubroutines& subroutine) const {
	return support(ShaderEnum::GEOMETRY_SHADER, subroutine);
}

bool Nimbus::VAO::support(const ShaderEnum::FragmentSubroutines& subroutine) const {
	return support(ShaderEnum::FRAGMENT_SHADER, subroutine);
}

bool Nimbus::VAO::support(const ShaderEnum::ShaderTypes& shaderType, const uint8_t& subroutine) const {
	switch (shaderType) {
	case ShaderEnum::VERTEX_SHADER:
		switch (subroutine) {
		case ShaderEnum::VERTEX_COLOR_SOURCE:
			return _defineVBO[VBO_COLOR_RGB] && (_defineVBO[VBO_THERMAL] || _defineVBO[VBO_MULTISPECTRAL] || _defineVBO[VBO_HYPERSPECTRAL]);
		default:
			return false;
		}
	case ShaderEnum::GEOMETRY_SHADER:
		switch (subroutine) {
		case ShaderEnum::DRAW_NORMALS:
			return _defineVBO[VBO_NORMAL];
		default:
			return false;
		}
	case ShaderEnum::FRAGMENT_SHADER:
		switch (subroutine) {
		case ShaderEnum::LIGHTING:
			return _defineVBO[VBO_NORMAL];
		case ShaderEnum::LIGHT_ATTENUATION:
			return _defineVBO[VBO_NORMAL];
		case ShaderEnum::FRAGMENT_COLOR_SOURCE:
			return _defineVBO[VBO_COLOR_RGB];
		default:
			return false;
		}
	default:
		return false;
	}
}


// Private methods

void Nimbus::VAO::defineNonInterleaveVBO(const uint32_t vboId, const size_t structSize, const uint32_t elementType, const VBO_slots slot) {
	glVertexArrayVertexBuffer(_vao, slot, vboId, 0, structSize); // VAO, bind del VAO, id VBO
	glEnableVertexArrayAttrib(_vao, slot); // VAO, atribIndex (El que se usa en el shader)
	glVertexArrayAttribBinding(_vao, slot, slot); // VAO, atribIndex (El que se usa en el shader), bind del VAO
	glVertexArrayAttribFormat(_vao, slot, static_cast<GLsizei>(structSize / sizeof(elementType)), elementType, GL_FALSE, 0);
	_defineVBO[slot] = true;
}

void Nimbus::VAO::defineInterleavedVBOForTriangleMesh() {
	if (_vbos[VBO_POSITION] == UINT32_MAX) {
		glCreateBuffers(1, &_vbos[VBO_POSITION]);
	}
	constexpr GLsizei structSize = sizeof(Vertex);
	glVertexArrayVertexBuffer(_vao, 0, _vbos[VBO_POSITION], 0, structSize);

	glEnableVertexArrayAttrib(_vao, VBO_POSITION);
	glVertexArrayAttribBinding(_vao, VBO_POSITION, 0);
	glVertexArrayAttribFormat(_vao, VBO_POSITION, static_cast<GLsizei>(sizeof(vec3) / sizeof(GL_FLOAT)), GL_FLOAT, GL_FALSE, offsetof(Vertex, _position));
	_defineVBO[VBO_POSITION] = true;

	glEnableVertexArrayAttrib(_vao, VBO_NORMAL);
	glVertexArrayAttribBinding(_vao, VBO_NORMAL, 0);
	glVertexArrayAttribFormat(_vao, VBO_NORMAL, static_cast<GLsizei>(sizeof(vec3) / sizeof(GL_FLOAT)), GL_FLOAT, GL_FALSE, offsetof(Vertex, _normal));
	_defineVBO[VBO_NORMAL] = true;

	glEnableVertexArrayAttrib(_vao, VBO_TEXT_COORD);
	glVertexArrayAttribBinding(_vao, VBO_TEXT_COORD, 0);
	glVertexArrayAttribFormat(_vao, VBO_TEXT_COORD, static_cast<GLsizei>(sizeof(vec2) / sizeof(GL_FLOAT)), GL_FLOAT, GL_FALSE, offsetof(Vertex, _textCoord));
	_defineVBO[VBO_TEXT_COORD] = true;
}

void Nimbus::VAO::defineInterleavedVBOForPointCloud() {
	if (_vbos[VBO_POSITION] == UINT32_MAX) {
		glCreateBuffers(1, &_vbos[VBO_POSITION]);
	}
	constexpr GLsizei structSize = sizeof(Point);
	glVertexArrayVertexBuffer(_vao, 0, _vbos[VBO_POSITION], 0, structSize);

	glEnableVertexArrayAttrib(_vao, VBO_POSITION);
	glVertexArrayAttribBinding(_vao, VBO_POSITION, 0);
	glVertexArrayAttribFormat(_vao, VBO_POSITION, static_cast<GLsizei>(sizeof(vec3) / sizeof(GL_FLOAT)), GL_FLOAT, GL_FALSE, offsetof(Point, _point));
	_defineVBO[VBO_POSITION] = true;

	glEnableVertexArrayAttrib(_vao, VBO_COLOR_RGB);
	glVertexArrayAttribBinding(_vao, VBO_COLOR_RGB, 0);
	glVertexArrayAttribFormat(_vao, VBO_COLOR_RGB, static_cast<GLsizei>(sizeof(unsigned) / sizeof(GL_FLOAT)), GL_FLOAT, GL_FALSE, offsetof(Point, _rgb));
	_defineVBO[VBO_COLOR_RGB] = true;
}
