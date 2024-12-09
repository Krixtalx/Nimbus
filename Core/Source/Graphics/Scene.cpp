#include "CorePch.h"
#include "Scene.h"

#include "DataTypes/PointCloud.h"


Nimbus::Scene::Scene(uint16_t width, uint16_t height) {
	_camera.push_back(std::make_unique<Camera>(width, height));
	_light.push_back(std::make_unique<Light>());
}

Nimbus::Scene::Scene(const ivec2 viewportSize) : Scene(viewportSize.x, viewportSize.y) {}

Nimbus::Scene::~Scene() {
	_camera.clear();
	_light.clear();
}

bool Nimbus::Scene::save() {
	if (!_scenePath.empty() && !_sceneName.empty()) {
		save(_scenePath, _sceneName);
		return true;
	}
	return false;
}

void Nimbus::Scene::save(const std::string& scenePath, const std::string& sceneName) {

	const std::string sceneFolderPath = scenePath + "\\" + sceneName;
	if (!std::filesystem::is_directory(sceneFolderPath) || !std::filesystem::exists(sceneFolderPath))
		std::filesystem::create_directory(sceneFolderPath);
	std::ofstream fout(sceneFolderPath + "\\" + sceneName + ".Nimbus", std::ios::out | std::ios::binary);
	if (!fout.is_open()) throw std::runtime_error("Nimbus::Scene::save -> Cannot open file.");
	try {
		std::cout << "Starting save..." << std::endl;		

		//Se guardan las nubes de puntos
		size_t numClouds = _pointClouds.size();
		fout.write(reinterpret_cast<char*>(&numClouds), sizeof(size_t));
		for (const auto& cloud : _pointClouds) {
			#ifdef TESTING
			cloud.second->saveBinaryFile(sceneFolderPath + "\\" + cloud.first);
			#else
			std::thread newThread(&Nimbus::PointCloud::saveBinaryFile, cloud.second.get(), sceneFolderPath + "\\" + cloud.first);
			newThread.detach();
			#endif
			size_t stringSize = cloud.first.size();
			fout.write(reinterpret_cast<char*>(&stringSize), sizeof(stringSize));
			fout.write(cloud.first.c_str(), stringSize);
}

		//Se guardan las camaras
		writeCameraArray(_camera, sceneFolderPath + "\\" + sceneName + ".NimbusCamera");

		//Se guardan las luces
		writeLightArray(_light, sceneFolderPath + "\\" + sceneName + ".NimbusLight");

		} catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
		fout.close();

		_sceneName = sceneName;
		_scenePath = scenePath;
		dirty = false;
		std::cout << "Finishing save..." << std::endl;
}

void Nimbus::Scene::load(const std::string& scenePath, const std::string& sceneName) {
	const std::string path = scenePath + "/";
	std::vector<std::thread> allThreads;

	std::ifstream fin(path + sceneName + ".Nimbus", std::ios::in | std::ios::binary);
	if (!fin.is_open()) throw std::runtime_error("Nimbus::Scene::save -> Cannot open file.");
	//std::cout << "Starting load..." << std::endl;

	//std::cout << "Loading point clouds..." << std::endl;
	//Se cargan las nubes de puntos
	size_t numClouds = 0;
	fin.read(reinterpret_cast<char*>(&numClouds), sizeof(size_t));
	allThreads.resize(numClouds);
	for (int i = 0; i < numClouds; ++i) {
		size_t stringSize = 0;
		fin.read(reinterpret_cast<char*>(&stringSize), sizeof(size_t));
		std::string filename;
		filename.resize(stringSize);
		fin.read(filename.data(), stringSize);
		auto pointCloud(std::make_unique<PointCloud>(filename));
		#ifdef TESTING
		pointCloud->loadBinaryFile(path + filename);
		#else
		allThreads.emplace_back(&Nimbus::PointCloud::loadBinaryFile, pointCloud.get(), path + filename);
		allThreads.back().detach();
		#endif
		_pointClouds.insert(std::make_pair(filename, std::move(pointCloud)));
	}

	//std::cout << "Loading cameras..." << std::endl;
	//Se cargan las cámaras
	readCameraArray(_camera, path + sceneName + ".NimbusCamera");

	//std::cout << "Loading lights..." << std::endl;
	//Se cargan las luces
	readLightArray(_light, path + sceneName + ".NimbusLight");

	fin.close();

	_sceneName = sceneName;
	_scenePath = scenePath;
	dirty = false;
	//std::cout << "Finishing load..." << std::endl;

}

Nimbus::Camera* Nimbus::Scene::getActiveCamera() const {
	return _camera[_activeCamera].get();
}

bool Nimbus::Scene::readCameraArray(std::vector<std::unique_ptr<Camera>>& buffer, const std::string& filename) const {
	std::ifstream fin(filename, std::ios::in | std::ios::binary);
	if (!fin.is_open()) return false;

	const size_t size = buffer.size();
	fin.read((char*)&size, sizeof(size_t));

	for (size_t idx = 0; idx < size; ++idx) {
		Camera* element = new Camera;
		delete element->_backupCamera;
		fin.read((char*)element, sizeof(Camera));
		element->_backupCamera = nullptr;
		element->setBackupCamera();

		buffer.push_back(std::make_unique<Camera>(*element));
	}

	fin.close();

	return true;
}

bool Nimbus::Scene::writeCameraArray(const std::vector<std::unique_ptr<Camera>>& buffer, const std::string& filename) {
	std::ofstream fout(filename, std::ios::out | std::ios::binary);
	if (!fout.is_open()) return false;

	const size_t size = buffer.size();
	fout.write((char*)&size, sizeof(size_t));

	for (const std::unique_ptr<Camera>& value : buffer) {
		fout.write(reinterpret_cast<char*>(value.get()), sizeof(Camera));
	}
	fout.close();

	return true;
}

bool Nimbus::Scene::readLightArray(std::vector<std::unique_ptr<Light>>& buffer, const std::string& filename) const {
	std::ifstream fin(filename, std::ios::in | std::ios::binary);
	if (!fin.is_open()) return false;

	const size_t size = buffer.size();
	fin.read((char*)&size, sizeof(size_t));

	for (size_t idx = 0; idx < size; ++idx) {
		Light* element = new Light;
		fin.read(reinterpret_cast<char*>(element), sizeof(Light));
		buffer.push_back(std::make_unique<Light>(*element));
	}

	fin.close();

	return true;
}

bool Nimbus::Scene::writeLightArray(const std::vector<std::unique_ptr<Light>>& buffer, const std::string& filename) {
	std::ofstream fout(filename, std::ios::out | std::ios::binary);
	if (!fout.is_open()) return false;

	const size_t size = buffer.size();
	fout.write((char*)&size, sizeof(size_t));

	for (const std::unique_ptr<Light>& value : buffer) {
		fout.write((char*)value.get(), sizeof(Light));
	}
	fout.close();

	return true;
}