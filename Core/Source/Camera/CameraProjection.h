#pragma once

namespace Nimbus {
	class CameraProjection {
	protected:
		static std::vector<std::shared_ptr<CameraProjection>> _cameraProjection;

	public:
		enum Projection {
			PERSPECTIVE, ORTHOGRAPHIC
		};

		class CameraProperties {
		public:
			int								_cameraType;

			dvec3							_eye, _lookAt, _up;
			float							_zNear, _zFar;
			float							_aspect;
			float							_fovY, _fovX;
			vec2							_bottomLeftCorner;
			uint16_t						_width, _height;
			dvec3							_n, _u, _v;
			dmat4							_viewMatrix, _projectionMatrix, _viewProjectionMatrix;

			float	computeAspect() const;
			void	computeAxes(dvec3& n, dvec3& u, dvec3& v) const;
			vec2	computeBottomLeftCorner() const;
			float	computeFovY() const;

			void	computeProjectionMatrices(CameraProperties* camera);
			void	computeViewMatrices();
			void	computeViewMatrix();

			void	zoom(float speed);

			void	setViewMatrix(const float* vMatrix);
			void	setProjectionMatrix(const float* pMatrix);
			float	getDistToLookAt() const;
		};

	public:
		virtual dmat4 buildProjectionMatrix(CameraProperties* camera) = 0;
		virtual void zoom(CameraProperties* camera, float speed) = 0;
	};

	class PerspectiveProjection : public CameraProjection {
	public:
		dmat4 buildProjectionMatrix(CameraProperties* camera) override;
		void zoom(CameraProperties* camera, float speed) override;
	};

	class OrthographicProjection : public CameraProjection {
	public:
		dmat4 buildProjectionMatrix(CameraProperties* camera) override;
		void zoom(CameraProperties* camera, float speed) override;
	};
}

