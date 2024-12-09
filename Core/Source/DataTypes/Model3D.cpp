#include "CorePch.h"
#include "Model3D.h"

Nimbus::Model3D::Model3D(const std::string& name, const vec3& pos, const vec3& rot, const vec3& scale, const vec3& offset) :
	_name(name), _pos(pos), _rot(rot), _scale(scale), _offset(offset), _enabled(true), _material(nullptr) {
}

Nimbus::Model3D::Model3D(const std::string& name, const AABB& aabb, const vec3& pos, const vec3& rot, const vec3& scale, const vec3& offset) :
	_name(name), _pos(pos), _rot(rot), _scale(scale), _offset(offset), _enabled(true), _aabb(aabb), _material(nullptr) {
}

void Nimbus::Model3D::move(const vec3& offset) {
	_pos += offset;
}

void Nimbus::Model3D::rotate(const vec3& offset) {
	_rot += offset;
}

void Nimbus::Model3D::scale(const vec3& offset) {
	_scale += offset;
}

void Nimbus::Model3D::setPosition(const vec3& newPosition) {
	_pos = newPosition;
}

void Nimbus::Model3D::setRotation(const vec3& newRotation) {
	_rot = newRotation;
}

void Nimbus::Model3D::setScale(const vec3& newScale) {
	_scale = newScale;
}

void Nimbus::Model3D::setOffset(const vec3& newOffset)
{
	_offset = newOffset;
}

void Nimbus::Model3D::setEnable(const bool enable) {
	_enabled = enable;
}

void Nimbus::Model3D::setAABB(const AABB& newAABB) {
	_aabb = newAABB;
}


void Nimbus::Model3D::moveGeometryToOrigin() {
	_pos = -_aabb.center();
}

dmat4 Nimbus::Model3D::getModelMatrix() const {
	dmat4 matrix(1.0f);
	const auto center = _aabb.center();
	matrix = translate(matrix, _pos + center);
	matrix = glm::rotate(matrix, radians(_rot.x), { 1, 0, 0 });
	matrix = glm::rotate(matrix, radians(_rot.y), { 0, 1, 0 });
	matrix = glm::rotate(matrix, radians(_rot.z), { 0, 0, 1 });
	matrix = glm::scale(matrix, _scale);
	matrix = translate(matrix, -center);
	return matrix;
}

dmat4 Nimbus::Model3D::getMVPMatrix(const dmat4& vpMatrix) const {
	return vpMatrix * getModelMatrix();
}

AABB Nimbus::Model3D::getAABB() {
	return _aabb.dot(getModelMatrix());
}

std::string Nimbus::Model3D::getName() const {
	return _name;
}

Nimbus::Material* Nimbus::Model3D::getMaterial() const {
	return _material;
}

dvec3 Nimbus::Model3D::getPosition() const {
	return _pos;
}

dvec3 Nimbus::Model3D::getOffset() const
{
	return _offset;
}
