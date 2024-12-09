#pragma once
#include "Camera/Camera.h"
#include "Graphics/Lights/Light.h"


namespace Nimbus {
	class PointCloud;
	class TriangleMesh;

	class Scene {
	public:
		std::string _sceneName = "New Scene";
		std::string _scenePath;
		bool dirty = false;

		unsigned _activeCamera = 0;
		std::vector<std::unique_ptr<Camera>>	_camera;
		std::vector<std::unique_ptr<Light>>		_light;
		std::unordered_map<std::string, std::unique_ptr<PointCloud>> _pointClouds;

		Scene() = delete;
		Scene(uint16_t width, uint16_t height);
		Scene(ivec2 viewportSize);
		virtual ~Scene();

		bool save();
		void save(const std::string& scenePath, const std::string& sceneName);
		void load(const std::string& scenePath, const std::string& sceneName);

		Camera* getActiveCamera() const;

		bool readCameraArray(std::vector<std::unique_ptr<Camera>>& buffer, const std::string& filename) const;
		bool writeCameraArray(const std::vector<std::unique_ptr<Camera>>& buffer, const std::string& filename);
		bool readLightArray(std::vector<std::unique_ptr<Light>>& buffer, const std::string& filename) const;
		bool writeLightArray(const std::vector<std::unique_ptr<Light>>& buffer, const std::string& filename);
	};
}
