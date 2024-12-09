#pragma once
#include "FBORender.h"
#include "FBOScreenshot.h"
#include "GPUResources.h"

#include "Scene.h"
#include "Managers/InputManager.h"
#include "efsw/efsw.hpp"

namespace Nimbus {
	class Application;
	class ComputeShader;
	class RenderingShader;
	class GUI;

	class Renderer : public Singleton<Renderer>, public ResizeListener, public ScreenshotListener, public efsw::FileWatchListener {
		friend Singleton;
		friend GUI;
		friend Application;

		ApplicationState* _appState;
		std::vector<std::unique_ptr<Scene>> _scenes;
		unsigned _activeScene;
		FBOScreenshot* _screenshoter;
		std::unique_ptr<FBORender> _renderFBO;
		RenderingShader* _triangleMeshShaderProgram;
		RenderingShader* _pointCloudShaderProgram;
		RenderingShader* _gridShaderProgram;
		RenderingShader* _meshletsAABBDrawProgram;
		ComputeShader* _resetDepthBuffer;
		ComputeShader* _meshletCullingShader;
		ComputeShader* _compactPositionsBuffer;
		ComputeShader* _copyPositionsShader;
		ComputeShader* _computeDepthBufferShader;
		//ComputeShader* _copyDepthBufferShader;
		ComputeShader* _composeImageShader;
		ComputeShader* _EDLShader;
		GPUResources _GPUResources;

		struct PointWithOffset {
			vec3 position;
			u32 offset;
			u32 attribute;
		};

		std::vector<PointWithOffset> _pointDataToSend;

		i32 _newPointsBufferSize;
		u32 _pointsPerSubBuffer;
		u8 _numBuffers;
		u8 _currentBufferId;
		u8 _prevBufferId;

		GLint	alignment;

		u32 _gridVAO;

		bool _shadersLoaded = false;
		bool _needToReloadComputeShaders = false;
		bool _zoomOutAnim = false;

		u64 _frameNumber = 0;
		double _frametime = 0;

		efsw::FileWatcher _fileWatcher;

		Renderer();
		void updateDeltaTime() const;
		void loadComputeShaders();
		void loadRenderShaders();
		u32 generateCompactInfo(PointCloud* pc, u32& prevIndex, u32& currentIndex, u32& pointsToSend);

	public:
		~Renderer() override;

		//Render things
		void prepareOpenGL(u16 width, u16 height, ApplicationState* appState);
		void render();
		void resizeEvent(u16 width, u16 height) override;
		void screenshotEvent() override;
		void reloadShaders();

		//==========================
		//			Scene
		//==========================


		// - Adders
		unsigned addModel(Model3D* model); // newModel
		unsigned addLight() const;
		unsigned addCamera() const;

		// - Deleters
		void deletePointCloud(const std::string& cloudName) const; // deleteModel 
		void deleteTriangleMesh(const std::string& meshName) const;
		void deleteLight(unsigned lightIdx) const;
		void deleteCamera(unsigned cameraIdx) const;
		bool deleteScene();

		// - Getters
		PointCloud* getPointCloud(const std::string& cloudName) const; // getModel
		TriangleMesh* getTriangleMesh(const std::string& meshName) const;
		Camera* getCamera() const { return _scenes[_activeScene]->getActiveCamera(); }
		bool sceneDirty() const { return _scenes[_activeScene]->dirty; }
		std::string getSceneCompleteName() const { return _scenes[_activeScene]->_scenePath + "/" + _scenes[_activeScene]->_sceneName; }
		std::string getSceneName() const { return _scenes[_activeScene]->_sceneName; }
		unsigned getCurrentActiveScene() const { return _activeScene; }
		Scene* getActiveScene() const { return _scenes[_activeScene].get(); }
		ApplicationState* getAppState() const {
			return _appState;
		}
		FBORender* getRenderFBO() const;

		// - Others

		void changeActiveScene(const unsigned& newActiveScene);
		Scene* newScene();
		void saveScene(const std::string& filepath, const std::string& filename) const;
		void loadScene(const std::string& filepath, const std::string& filename);

		void handleFileAction(efsw::WatchID watchid, const std::string& dir,
							  const std::string& filename, efsw::Action action,
							  std::string oldFilename) override;

		void zoomOutAnim() { _zoomOutAnim = !_zoomOutAnim; }
	};
}

