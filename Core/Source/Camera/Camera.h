#pragma once

#include "CameraProjection.h"
#include "DataTypes/Model3D.h"

namespace Nimbus {
	class Camera {
		friend class DetailsWindow;
		friend class CameraAxisWindow;
		friend class Scene;

	protected:
		Camera* _backupCamera = nullptr;
		CameraProjection::CameraProperties	_properties;

		std::string _name;
		Model3D* trackedModel = nullptr;

		void copyCameraAttributes(const Camera* camera);
		void setBackupCamera();

	public:
		bool changed = true;

		Camera();
		Camera(uint16_t width, uint16_t height);
		Camera(const Camera& camera);
		virtual ~Camera();
		void reset();
		void lookPosition(const vec3& position);
		void lookModel(Model3D* model);
		void trackModel(Model3D* model);
		void untrackModel();


		Camera& operator=(const Camera& camera) = delete;

		dmat4 getViewMatrix() const {
			return _properties._viewMatrix;
		}
		dmat4 getProjectionMatrix() const {
			return _properties._projectionMatrix;
		}
		dmat4 getViewProjectionMatrix() const {
			return _properties._viewProjectionMatrix;
		}
		vec3 getCameraPosition() const {
			return _properties._eye;
		}
		Model3D* getTrackedModel() const;
		float getZnear() const
		{
			return _properties._zNear;
		}
		float getZfar() const
		{
			return _properties._zFar;
		}
		float getFoV() const
		{
			return _properties._fovX;
		}

		void saveCamera();
		void setBottomLeftCorner(const vec2& bottomLeft);
		void setCameraType(CameraProjection::Projection projection);
		void setFovX(float fovX);
		void setFovY(float fovY);
		void setLookAt(const vec3& position);
		void setPosition(const vec3& position);
		void setRaspect(uint16_t width, uint16_t height);
		void setRaspect(const vec2& rasp);
		void setUp(const vec3& up);
		void setZFar(float zfar);
		void setZNear(float znear);
		void updateMatrices();

		// Movements

		void boom(double speed);
		void crane(double speed);
		void dolly(double speed);
		void orbitXZ(double speed);
		void orbitY(double speed);
		void pan(double speed);
		void tilt(double speed);
		void truck(double speed);
		void zoom(double speed);
		void rotate(double speed);

		std::string getName() {
			return _name;
		}
		void setName(const std::string& name) {
			_name = name;
		}
		vec2 getRaspect() const {
			return { _properties._width, _properties._height };
		}

		CameraProjection::CameraProperties& getProperties()
		{
			return _properties;
		}

		vec2 projectPoint(vec4 point) const;
		vec2 projectPointNorm(vec4 point) const;

		void makeViewRay(vec2 screenPoint, uvec2 viewportSize, dvec3& origin, dvec3& direction);
	};
}

