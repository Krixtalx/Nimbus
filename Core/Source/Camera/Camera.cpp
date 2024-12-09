#include "CorePch.h"
#include "Camera.h"

#include <glm/ext/matrix_projection.hpp>

#include "CameraProjection.h"
#include "DataTypes/Geometry/AABB.h"


// Public methods

Nimbus::Camera::Camera() : Camera(1024, 576) {}

Nimbus::Camera::Camera(const uint16_t width, const uint16_t height) : _name("New camera") {
	this->_properties._cameraType = CameraProjection::PERSPECTIVE;

	this->_properties._eye = vec3(3.0f, 3.0f, 3.0f);
	this->_properties._lookAt = vec3(0.0f, 0.0f, 0.0f);
	//this->_properties._eye = vec3(0.0f, 3.0f, 10.0f);
	//this->_properties._lookAt = vec3(0.0f, 3.0f, 0.0f);
	this->_properties._up = vec3(0.0f, 1.0f, 0.0f);

	this->_properties._zNear = 0.1f;
	this->_properties._zFar = 6000.0f;

	this->_properties._width = width;
	this->_properties._height = height;
	this->_properties._aspect = this->_properties.computeAspect();

	this->_properties._bottomLeftCorner = vec2(-2.0f * this->_properties._aspect, -2.0f);
	this->_properties._fovX = 80.0f * glm::pi<float>() / 180.0f;
	this->_properties._fovY = this->_properties.computeFovY();

	this->_properties.computeAxes(this->_properties._n, this->_properties._u, this->_properties._v);
	this->_properties.computeViewMatrix();
	this->_properties.computeProjectionMatrices(&this->_properties);

	this->saveCamera();
}

Nimbus::Camera::Camera(const Nimbus::Camera& camera) {
	this->copyCameraAttributes(&camera);
}

Nimbus::Camera::~Camera() {
	delete _backupCamera;
}

void Nimbus::Camera::reset() {
	this->copyCameraAttributes(_backupCamera);
	changed = true;
}

void Nimbus::Camera::lookPosition(const vec3& position) {
	const vec3 movVec = (dvec3)position - this->_properties._lookAt;
	this->setLookAt(position);
	this->setPosition(this->_properties._eye + (dvec3)movVec);

	changed = true;
}

void Nimbus::Camera::lookModel(Model3D* model) {
	const AABB aabb = model->getAABB();
	vec3 pos = aabb.max();
	pos.y += 20;
	this->setPosition(pos);
	this->setLookAt(aabb.center());
	changed = true;
}

void Nimbus::Camera::trackModel(Model3D* model) {
	trackedModel = model;
}

void Nimbus::Camera::untrackModel() {
	trackedModel = nullptr;
}

Nimbus::Model3D* Nimbus::Camera::getTrackedModel() const {
	return trackedModel;
}

void Nimbus::Camera::saveCamera() {
	delete _backupCamera;

	_backupCamera = new Nimbus::Camera(*this);
}

void Nimbus::Camera::setBottomLeftCorner(const vec2& bottomLeft) {
	this->_properties._bottomLeftCorner = bottomLeft;
	this->_properties.computeProjectionMatrices(&this->_properties);
	changed = true;
}

void Nimbus::Camera::setCameraType(const Nimbus::CameraProjection::Projection projection) {
	this->_properties._cameraType = projection;
	this->_properties.computeViewMatrices();
	this->_properties.computeProjectionMatrices(&this->_properties);

	changed = true;
}

void Nimbus::Camera::setFovX(const float fovX) {
	this->_properties._fovX = fovX;
	this->_properties._fovY = this->_properties.computeFovY();
	this->_properties.computeProjectionMatrices(&this->_properties);

	changed = true;
}

void Nimbus::Camera::setFovY(const float fovY) {
	this->_properties._fovY = fovY;
	this->_properties.computeProjectionMatrices(&this->_properties);

	changed = true;
}

void Nimbus::Camera::setLookAt(const vec3& position) {
	this->_properties._lookAt = position;
	this->_properties.computeAxes(this->_properties._n, this->_properties._u, this->_properties._v);
	this->_properties.computeViewMatrices();

	changed = true;
}

void Nimbus::Camera::setPosition(const vec3& position) {
	this->_properties._eye = position;
	this->_properties.computeAxes(this->_properties._n, this->_properties._u, this->_properties._v);
	this->_properties.computeViewMatrices();

	changed = true;
}

void Nimbus::Camera::setRaspect(const uint16_t width, const uint16_t height) {
	this->_properties._width = width;
	this->_properties._height = height;
	this->_properties._aspect = this->_properties.computeAspect();
	this->_properties._bottomLeftCorner = vec2(this->_properties._bottomLeftCorner.y * this->_properties._aspect, this->_properties._bottomLeftCorner.y);
	this->_properties.computeProjectionMatrices(&this->_properties);

	changed = true;
}

void Nimbus::Camera::setRaspect(const vec2& rasp) {
	setRaspect(rasp.x, rasp.y);
}

void Nimbus::Camera::setUp(const vec3& up) {
	this->_properties._up = up;
	this->_properties.computeViewMatrices();
	changed = true;
}

void Nimbus::Camera::setZFar(const float zfar) {
	this->_properties._zFar = zfar;
	this->_properties.computeProjectionMatrices(&this->_properties);

	changed = true;
}

void Nimbus::Camera::setZNear(const float znear) {
	this->_properties._zNear = znear;
	this->_properties.computeProjectionMatrices(&this->_properties);
	
	changed = true;
}

void Nimbus::Camera::updateMatrices() {
	this->_properties.computeViewMatrix();
	this->_properties.computeProjectionMatrices(&_properties);
}

// [Movements] 

void Nimbus::Camera::boom(const double speed) {
	const glm::mat4 translationMatrix = glm::translate(dmat4(1.0f), this->_properties._v * speed);			// Translation in y axis

	this->_properties._eye = vec3(translationMatrix * vec4(this->_properties._eye, 1.0f));
	this->_properties._lookAt = vec3(translationMatrix * vec4(this->_properties._lookAt, 1.0f));

	this->_properties.computeViewMatrices();
	changed = true;
}

void Nimbus::Camera::crane(const double speed) {
	boom(-speed);					// Implemented as another method to take advantage of nomenclature
}

void Nimbus::Camera::dolly(const double speed) {
	const mat4 translationMatrix = glm::translate(dmat4(1.0f), -this->_properties._n * speed);			// Translation in z axis
	this->_properties._eye = vec3(translationMatrix * vec4(this->_properties._eye, 1.0f));
	this->_properties._lookAt = vec3(translationMatrix * vec4(this->_properties._lookAt, 1.0f));

	this->_properties.computeViewMatrices();
	changed = true;
}

void Nimbus::Camera::orbitXZ(const double speed) {
	const mat4 rotationMatrix = glm::rotate(dmat4(1.0f), speed, this->_properties._u);					// We will pass over the scene, x or z axis could be used


	this->_properties._eye = dvec3(rotationMatrix * dvec4(this->_properties._eye - this->_properties._lookAt, 1.0f)) + this->_properties._lookAt;
	this->_properties._u = dvec3(rotationMatrix * vec4(this->_properties._u, 0.0f));
	this->_properties._v = dvec3(rotationMatrix * vec4(this->_properties._v, 0.0f));
	this->_properties._n = dvec3(rotationMatrix * vec4(this->_properties._n, 0.0f));
	this->_properties._up = glm::normalize(glm::cross(this->_properties._n, this->_properties._u));						// Free rotation => we can look down or up

	// if (trackedModel)
	// 	setLookAt(trackedModel->getAABB().center());

	this->_properties.computeViewMatrices();
	changed = true;
}

void Nimbus::Camera::orbitY(const double speed) {
	const mat4 rotationMatrix = glm::rotate(dmat4(1.0f), speed, this->_properties._v);

	this->_properties._eye = dvec3(rotationMatrix * vec4(this->_properties._eye - this->_properties._lookAt, 1.0f)) + this->_properties._lookAt;
	this->_properties._u = dvec3(rotationMatrix * vec4(this->_properties._u, 0.0f));
	this->_properties._v = dvec3(rotationMatrix * vec4(this->_properties._v, 0.0f));
	this->_properties._n = dvec3(rotationMatrix * vec4(this->_properties._n, 0.0f));
	this->_properties._up = glm::normalize(glm::cross(this->_properties._n, this->_properties._u));								// This movement doesn't change UP, but it could occur as a result of previous operations

	// if (trackedModel)
	// 	setLookAt(trackedModel->getAABB().center());

	this->_properties.computeViewMatrices();
	changed = true;
}

void Nimbus::Camera::pan(const double speed) {
	const dmat4 rotationMatrix = glm::rotate(dmat4(1.0f), speed, this->_properties._v);

	// Up vector can change, not in the original position tho. Example: orbit XZ (rotated camera) + pan
	this->_properties._u = dvec3(rotationMatrix * vec4(this->_properties._u, 0.0f));
	this->_properties._v = dvec3(rotationMatrix * vec4(this->_properties._v, 0.0f));
	this->_properties._n = dvec3(rotationMatrix * vec4(this->_properties._n, 0.0f));
	this->_properties._up = glm::normalize(glm::cross(this->_properties._n, this->_properties._u));
	this->_properties._lookAt = dvec3(rotationMatrix * vec4(this->_properties._lookAt - this->_properties._eye, 1.0f)) + this->_properties._eye;

	this->_properties.computeViewMatrices();
	changed = true;
}

void Nimbus::Camera::tilt(const double speed) {
	const mat4 rotationMatrix = glm::rotate(dmat4(1.0f), speed, this->_properties._u);

	const auto n = glm::vec3(rotationMatrix * glm::dvec4(this->_properties._n, 0.0f));
	const float alpha = glm::acos(glm::dot(n, glm::vec3(0.0f, 1.0f, 0.0f)));

	if (alpha < speed || alpha >(glm::pi<float>() - speed)) {
		return;
	}

	this->_properties._v = glm::dvec3(rotationMatrix * glm::dvec4(this->_properties._v, 0.0f));
	this->_properties._n = n;
	this->_properties._up = glm::normalize(glm::cross(this->_properties._n, this->_properties._u));											// It could change because of the rotation
	this->_properties._lookAt = dvec3(rotationMatrix * glm::dvec4(this->_properties._lookAt - this->_properties._eye, 1.0f)) + this->_properties._eye;

	this->_properties.computeViewMatrices();
	changed = true;
}

void Nimbus::Camera::truck(const double speed) {
	const mat4 translationMatrix = glm::translate(dmat4(1.0f), this->_properties._u * speed);				// Translation in x axis

	this->_properties._eye = vec3(translationMatrix * vec4(this->_properties._eye, 1.0f));
	this->_properties._lookAt = vec3(translationMatrix * vec4(this->_properties._lookAt, 1.0f));

	this->_properties.computeViewMatrices();
	changed = true;
}

void Nimbus::Camera::zoom(const double speed) {
	this->_properties.zoom(speed);
	changed = true;
}

void Nimbus::Camera::rotate(const double speed) {
	const auto rotationMatrix = glm::rotate(dmat4(1.0f), speed, _properties._n);
	_properties._v = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(this->_properties._v, 0.0f)));
	_properties._u = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(this->_properties._u, 0.0f)));
	this->_properties._up = glm::normalize(glm::cross(this->_properties._n, this->_properties._u));
	this->_properties.computeViewMatrices();
	changed = true;
}

vec2 Nimbus::Camera::projectPoint(const vec4 point) const {
	vec4 projPoint = _properties._viewProjectionMatrix * point;
	projPoint /= projPoint.w;
	projPoint = (projPoint + 1.0f) * 0.5f;
	projPoint.y = 1.0f - projPoint.y;
	// spdlog::info("proj: {}	{}	{}", projPoint.x, projPoint.y, projPoint.z);
	return vec2(projPoint) * vec2(_properties._width, _properties._height);
}

vec2 Nimbus::Camera::projectPointNorm(const vec4 point) const {
	vec4 projPoint = _properties._viewProjectionMatrix * point;
	projPoint /= projPoint.w;
	projPoint = (projPoint + 1.0f) * 0.5f;
	projPoint.y = 1.0f - projPoint.y;
	// spdlog::info("proj: {}	{}	{}", projPoint.x, projPoint.y, projPoint.z);
	return vec2(projPoint);
}

void Nimbus::Camera::makeViewRay(vec2 screenPoint, uvec2 viewportSize, dvec3& origin, dvec3& direction)
{
	// dvec3 point(screenPoint.x, viewportSize.y - screenPoint.y, getZnear());
	// auto camViewMat = getViewMatrix();
	// auto camProjMat = getProjectionMatrix();
	// dvec4 viewport({ 0.0, 0.0, viewportSize.x, viewportSize.y });
	// auto worldPoint = unProject(point, camViewMat, camProjMat, viewport);

	double pointScreenX = screenPoint.x / (viewportSize.x * 0.5) - 1.0;
	double pointScreenY = screenPoint.y / (viewportSize.y * 0.5) - 1.0;

	dvec4 point(pointScreenX, -pointScreenY, 1.0, 1.0);
	mat4 projectionMatrix = glm::perspective(_properties._fovY, _properties._aspect, _properties._zNear, _properties._zFar);
	mat4 viewMatrix = glm::lookAt(dvec3(getCameraPosition()), _properties._lookAt, _properties._up);
	mat4 inverseProjectionView = inverse(projectionMatrix * viewMatrix);
	dvec4 worldPoint = inverseProjectionView * point;

	origin = getCameraPosition();
	// direction = normalize(worldPoint - origin);
	direction = normalize(vec3(worldPoint));
}


/// [Private methods]

void Nimbus::Camera::copyCameraAttributes(const Nimbus::Camera* camera) {
	this->_properties = camera->_properties;

	if (camera->_backupCamera) {
		delete this->_backupCamera;
		this->_backupCamera = new Nimbus::Camera(*camera->_backupCamera);
	}
}

void Nimbus::Camera::setBackupCamera() {
	this->_backupCamera = new Nimbus::Camera();

	delete this->_backupCamera->_backupCamera;
	this->_backupCamera->_backupCamera = nullptr;

	this->_backupCamera->_properties = this->_properties;
}
