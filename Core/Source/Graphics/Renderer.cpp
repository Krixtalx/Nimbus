#include "CorePch.h"
#include "Renderer.h"

#include "ApplicationState.h"
#include "MaterialList.h"
#include "DataTypes/PointCloud.h"
#include "Managers/MemoryManager.h"
#include "Shaders/ComputeShader.h"
#include "Shaders/RenderingShader.h"
#include "Shaders/ShaderManager.h"

/**
 * \brief Callback mensajes de debug de OpenGL.
 */
void message_callback(GLenum source, GLenum type, u32 id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param) {
	auto const src_str = [source]() {
		switch (source) {
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
		default: return "UNDEFINED";
		}
	}();

	auto const type_str = [type]() {
		switch (type) {
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER: return "MARKER";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		default: return "UNDEFINED";
		}
	}();

	auto const severity_str = [severity]() {
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		default: return "UNDEFINED";
		}
	}();
	if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR) {
		spdlog::error("[{},{},{}]: ID:{} \n {}", src_str, type_str, severity_str, id, message);
	} else if (type == GL_DEBUG_TYPE_PERFORMANCE || severity == GL_DEBUG_SEVERITY_MEDIUM) {
		spdlog::warn("[{},{},{}]: ID:{} \n {}", src_str, type_str, severity_str, id, message);
	} else {
		//spdlog::info("[{},{},{}]: ID:{} \n {}", src_str, type_str, severity_str, id, message);
	}
}

// Public methods

Nimbus::Renderer::Renderer() : _appState(nullptr), _activeScene(0), _screenshoter(nullptr),
_triangleMeshShaderProgram(nullptr), _pointCloudShaderProgram(nullptr),
_gridShaderProgram(nullptr), _resetDepthBuffer(nullptr), _meshletCullingShader(nullptr),
_computeDepthBufferShader(nullptr), _composeImageShader(nullptr),
_gridVAO(0) {
}

void Nimbus::Renderer::updateDeltaTime() const {
	_appState->currentFrameTime = glfwGetTime();
	_appState->deltaTime = (_appState->currentFrameTime - _appState->lastFrameTime);
	_appState->lastFrameTime = _appState->currentFrameTime;
}

void Nimbus::Renderer::loadComputeShaders() {
	_shadersLoaded = false;
	ShaderManager::getInstance()->clearComputeShaders();
	_meshletCullingShader = ShaderManager::getInstance()->getComputeShader("MeshletCulling");
	_compactPositionsBuffer = ShaderManager::getInstance()->getComputeShader("CompactPositionsBuffer");
	_copyPositionsShader = ShaderManager::getInstance()->getComputeShader("CopyPositions");
	_computeDepthBufferShader = ShaderManager::getInstance()->getComputeShader("ComputeDepthBuffer");
	//_copyDepthBufferShader = ShaderManager::getInstance()->getComputeShader("CopyDepthBuffer");
	_composeImageShader = ShaderManager::getInstance()->getComputeShader("ComposeImage");
	_EDLShader = ShaderManager::getInstance()->getComputeShader("EDL");
	_resetDepthBuffer = ShaderManager::getInstance()->getComputeShader("ResetDepthBuffer");
	_shadersLoaded = true;
}

void Nimbus::Renderer::loadRenderShaders() {
	_shadersLoaded = false;
	ShaderManager::getInstance()->clearRenderShaders();
	_triangleMeshShaderProgram = ShaderManager::getInstance()->getRenderingShader("TriangleMesh");
	_pointCloudShaderProgram = ShaderManager::getInstance()->getRenderingShader("PointCloud");
	const auto linesModelShaderProgram = ShaderManager::getInstance()->getRenderingShader("LinesModel");
	_meshletsAABBDrawProgram = ShaderManager::getInstance()->getRenderingShader("MeshletsAABB");
	_gridShaderProgram = ShaderManager::getInstance()->getRenderingShader("InfiniteGrid");

	MaterialList::getInstance()->saveMaterial("PointCloudMaterial", new Material("PointCloudMaterial", _pointCloudShaderProgram));
	MaterialList::getInstance()->saveMaterial("TriangleMeshMaterial", new Material("TriangleMeshMaterial", _triangleMeshShaderProgram));
	MaterialList::getInstance()->saveMaterial("LinesModelMaterial", new Material("LinesModelMaterial", linesModelShaderProgram));
	_shadersLoaded = true;
}

// Private methods

Nimbus::Renderer::~Renderer() {
	delete _screenshoter;
	glDeleteVertexArrays(1, &_gridVAO);
}

void Nimbus::Renderer::resizeEvent(const u16 width, const u16 height) {
	if (width == 0 || height == 0) {
		return;
	}

	glViewport(0, 0, width, height);

	_appState->_viewportSize = ivec2(width, height);
	_scenes[_activeScene]->getActiveCamera()->setRaspect(width, height);
	_renderFBO->modifySize(width, height);
	_GPUResources.reserveGPUMemory<uint64_t>(GPUResources::SSBOSlots::DepthBuffer, _appState->_viewportSize.x * _appState->_viewportSize.y);
}

void Nimbus::Renderer::screenshotEvent() {
	const ivec2 size = _appState->_viewportSize;
	const ivec2 newSize = ivec2(_appState->_viewportSize.x * _appState->_screenshotFactor, _appState->_viewportSize.y * _appState->_screenshotFactor);

	_screenshoter->modifySize(newSize.x, newSize.y);
	_screenshoter->bindFramebuffer();
	this->resizeEvent(newSize.x, newSize.y);
	this->render();
	const bool success = _screenshoter->saveImage(_appState->_screenshotFilenameBuffer + std::to_string(_appState->deltaTime));

	this->resizeEvent(size.x, size.y);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Nimbus::Renderer::reloadShaders() {
	ShaderManager::getInstance()->clear();
	loadComputeShaders();
	loadRenderShaders();
}

void Nimbus::Renderer::prepareOpenGL(u16 width, u16 height, ApplicationState* appState) {
	_appState = appState;
	_appState->_viewportSize = ivec2(width, height);
	_scenes.push_back(std::make_unique<Scene>(width, height));
	_screenshoter = new FBOScreenshot(width, height);
	_renderFBO = std::make_unique<FBORender>(width, height);

	ComputeShader::initializeStaticVariables();

	// - Establecemos un gris medio como color con el que se borrar� el frame buffer.
	// No tiene por qu� ejecutarse en cada paso por el ciclo de eventos.
	glClearColor(_appState->_backgroundColor.x, _appState->_backgroundColor.y, _appState->_backgroundColor.z, 1.0f);

	// - Le decimos a OpenGL que tenga en cuenta la profundidad a la hora de dibujar.
	// No tiene por qu� ejecutarse en cada paso por el ciclo de eventos.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(RESTART_PRIMITIVE_INDEX);

	glEnable(GL_PROGRAM_POINT_SIZE);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glDebugMessageCallback(message_callback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	this->loadComputeShaders();
	this->loadRenderShaders();
	//this->createLights();

	// Observer
	InputManager* inputManager = InputManager::getInstance();
	inputManager->suscribeResize(this);
	inputManager->suscribeScreenshot(this);

	_fileWatcher.addWatch("Assets/Shaders/ComputeShaders", this);
	_fileWatcher.watch();

	{
		glGenVertexArrays(1, &_gridVAO);
		glBindVertexArray(_gridVAO);
	}

	glGetInteger64v(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &GPUResources::maxMemoryPerSSBO);
	glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &alignment);
	_newPointsBufferSize = 3000000;
	_pointBucket = 10000000;
	_pointsPerSubBuffer = GPUResources::maxMemoryPerSSBO / sizeof(glm::vec3);
	_pointDataToSend.resize(_newPointsBufferSize);
	_numBuffers = 2;
	_prevBufferId = 0;
	_currentBufferId = 1;
	_GPUResources.reserveGPUMemory<uint64_t>(GPUResources::SSBOSlots::DepthBuffer, static_cast<size_t>(_appState->_viewportSize.x) * _appState->_viewportSize.y, 0);
	_GPUResources.reserveGPUMappedMemory<u32>(GPUResources::SSBOSlots::ReadBackData, 1, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	_GPUResources.reserveGPUMemory<vec3>(GPUResources::SSBOSlots::PositionFrame1, _pointsPerSubBuffer, 0);
	_GPUResources.reserveGPUMemory<vec3>(GPUResources::SSBOSlots::PositionFrame2, _pointsPerSubBuffer, 0);
	_GPUResources.reserveGPUMemory<u32>(GPUResources::SSBOSlots::AttributeFrame1, _pointsPerSubBuffer, 0);
	_GPUResources.reserveGPUMemory<u32>(GPUResources::SSBOSlots::AttributeFrame2, _pointsPerSubBuffer, 0);
	_GPUResources.reserveGPUMappedMemory<PointWithOffset>(GPUResources::SSBOSlots::UploadBuffer, _newPointsBufferSize * _numBuffers, GL_MAP_WRITE_BIT);
}



u32 Nimbus::Renderer::generateCompactInfo(PointCloud* pc, u32& prevIndex, u32& currentIndex, u32& pointsToSend) {
	auto& pcGPUResources = pc->_GPUResources;
	const u32 numMeshlets = pc->getMeshletNumber();

	std::vector<PointCloud::CompactInfo> compactInfoForGPU;
	compactInfoForGPU.reserve(numMeshlets / 3);

	u16 maxPointsToMove = 0;
	pc->_numVisiblePoints = 0;

	pc->_GPUResources.waitForLockedRange(GPUResources::SSBOSlots::CullingBuffer);
	pc->_GPUResources.readGPUMappedMemory(GPUResources::SSBOSlots::CullingBuffer, pc->_currentFrameCulling.data(), numMeshlets);

	u32 pointsUsed = 0;
	for (auto& pointsToRender : pc->_currentFrameCulling) {
		if (pointsToRender > 0 && pointsToRender < pc->_meshletSize) {
			u32 temp = ((float)pointsToRender / _appState->_renderedPoints[_currentBufferId]) * _pointBucket;
			pointsToRender = std::min(temp, pc->_meshletSize);
			pointsUsed += pointsToRender;
		}
	}
	/*
	u32 remainingPoints = _pointBucket - pointsUsed;
	bool keepGoing = true;
	while (remainingPoints > 0 && keepGoing) {
		keepGoing = false;
		for (auto& pointsToRender : pc->_currentFrameCulling) {
			if (pointsToRender > 0 && pointsToRender < pc->_meshletSize) {
				u32 temp = pointsToRender + ((float)pointsToRender / _appState->_renderedPoints[_currentBufferId]) * remainingPoints;
				pointsToRender = std::min(temp, pc->_meshletSize);
				pointsUsed += pointsToRender;
				keepGoing = true;
			}
		}
		remainingPoints = _pointBucket - pointsUsed;
	}
	*/

	for (u32 i = 0; i < numMeshlets; ++i) {
		/*srand(i);
		vec4 randomColor = vec4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1);
		u32 randomColorAsUint = glm::packUnorm4x8(randomColor);*/

		const u16 loadedPointsInGPU = pc->_lastFrameCulling[i];
		const u16 maxLoadedPoints = std::max(loadedPointsInGPU, pc->_meshlets[i].pointsLoaded);
		const u16 requiredPoints = std::min(pc->_currentFrameCulling[i], maxLoadedPoints);

		const i32 pointsToUpload = requiredPoints - loadedPointsInGPU;
		const i32 bufferSizeLeft = _newPointsBufferSize - pointsToSend;
		pc->_meshlets[i].pointsRequired = pc->_currentFrameCulling[i];
		if (requiredPoints && loadedPointsInGPU) {
			u16 pointsToMove = std::min(requiredPoints, loadedPointsInGPU);
			compactInfoForGPU.emplace_back(prevIndex,
										   currentIndex,
										   pointsToMove);
			maxPointsToMove = std::max(maxPointsToMove, pointsToMove);
		}

		if (bufferSizeLeft > pointsToUpload) {
			for (i32 j = 0; j < pointsToUpload; ++j) {
				_pointDataToSend[pointsToSend++] = {
					pc->_meshlets[i].pos[loadedPointsInGPU + j],
					currentIndex + loadedPointsInGPU + j,
					pc->_meshlets[i].att[loadedPointsInGPU + j].asUint
					//randomColorAsUint
				};
			}

			pc->_lastFrameCulling[i] = requiredPoints;
		} else {
			pc->_lastFrameCulling[i] = std::min(requiredPoints, loadedPointsInGPU);
		}

		pc->_numVisiblePoints += pc->_lastFrameCulling[i];
		currentIndex += pc->_lastFrameCulling[i];
		prevIndex += loadedPointsInGPU;
	}
	if (!compactInfoForGPU.empty()) {
		//fmt::print("{} meshlets copied\n", compactInfoForGPU.size());
		pc->_numCompactInfo = compactInfoForGPU.size();
		pcGPUResources.writeGPUMappedData(GPUResources::SSBOSlots::CompactionInfo, compactInfoForGPU.data(), pc->_numCompactInfo);
		pc->_maxPointsToMove = maxPointsToMove;
		/*glCopyNamedBufferSubData(
			_GPUResources._ssboIds[(u8)GPUResources::SSBOSlots::Position],
			_GPUResources._ssboIds[(u8)GPUResources::SSBOSlots::Position],
			_pointsPerSubBuffer * _prevBufferId * sizeof(vec3),
			_pointsPerSubBuffer * _currentBufferId * sizeof(vec3),
			compactInfoForGPU.back().newIndex * sizeof(vec3)
		);
		glCopyNamedBufferSubData(
			_GPUResources._ssboIds[(u8)GPUResources::SSBOSlots::Attribute],
			_GPUResources._ssboIds[(u8)GPUResources::SSBOSlots::Attribute],
			_pointsPerSubBuffer * _prevBufferId * sizeof(u32),
			_pointsPerSubBuffer * _currentBufferId * sizeof(u32),
			compactInfoForGPU.back().newIndex * sizeof(u32)
		);*/
		//fmt::print("{} points to move per meshlet\n", maxPointsToMove);
	} else {
		pc->_numCompactInfo = 0;
	}

	return pointsToSend;
}

void Nimbus::Renderer::render() {
	if (_needToReloadComputeShaders) {
		_needToReloadComputeShaders = false;
		loadComputeShaders();
	}
	getCamera()->changed = true;
	if (_shadersLoaded && getCamera()->changed) {
		//fmt::println("{}", _frameNumber++);
		getCamera()->changed = false;
		//glClearColor(_appState->_backgroundColor.x, _appState->_backgroundColor.y, _appState->_backgroundColor.z, alpha);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDepthFunc(GL_LEQUAL);

		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 6, "Render");

		try {
			//static bool stop = false;
			//if (!_scenes[_activeScene]->_pointClouds.empty() && !stop) {
			//	static i32 sectors = 250;
			//	static i32 currentSector = 0;
			//	static i32 stacks = 125;
			//	static i32 currentStack = -1;
			//	static i32 numFrames = 1;
			//	static i32 radius = 3000;
			//	static ProgressBar bar(sectors * stacks * numFrames);
			//	static float sectorStep = 2 * M_PI / sectors;
			//	static float stackStep = 2 * M_PI / stacks;
			//	static float sectorAngle = 0, stackAngle = M_PI / 2.0f - stackStep;
			//	/*static bool firstFrame = true;
			//	if (!firstFrame) {
			//		spdlog::info("Sector: {}", currentSector);
			//		spdlog::info("Stack: {}", currentStack);
			//		spdlog::info("Frametime: {}", _appState->deltaTime);
			//		spdlog::info("ProcessedPoints: {}", _appState->_renderedPoints[_currentBufferId]);
			//	}
			//	firstFrame = false;*/

			//	if (!(_frameNumber % numFrames)) {
			//		float xy = radius * cosf(stackAngle);
			//		static vec3 pos;
			//		pos.z = radius * sinf(stackAngle);
			//		//pos = vec3(xy * cosf(sectorAngle), pos.z, xy * sinf(sectorAngle));
			//		pos = vec3(xy * cosf(sectorAngle), xy * sinf(sectorAngle), pos.z);
			//		pos += _scenes[_activeScene]->_pointClouds["1B.las"]->getAABB().center();
			//		_scenes[_activeScene]->getActiveCamera()->setPosition(pos);

			//		sectorAngle += sectorStep;
			//		currentSector++;
			//		if (!(_frameNumber % (numFrames * sectors))) {
			//			stackAngle -= stackStep;
			//			sectorAngle = 0;
			//			currentSector = 0;
			//			currentStack++;
			//		}
			//	}
			//	bar.update();
			//	_frameNumber++;
			//	if (_frameNumber > (sectors * stacks * numFrames)) {
			//		stop = true;
			//	}
			//}

			/*_scenes[_activeScene]->getActiveCamera()->orbitY(_appState->inputDeltaTime);
			_scenes[_activeScene]->getActiveCamera()->orbitXZ(_appState->inputDeltaTime * std::cos(_frametime));
			_scenes[_activeScene]->getActiveCamera()->zoom(_appState->inputDeltaTime * std::cos(_frametime)*0.5f);*/
			if (_zoomOutAnim) {
				++_frameNumber;
				_frametime += _appState->deltaTime;
				_scenes[_activeScene]->getActiveCamera()->truck(_appState->deltaTime*45);
				_scenes[_activeScene]->getActiveCamera()->zoom(_appState->deltaTime * std::cos(_frametime) * 0.25f);
			} else {
				_frameNumber = 0;
				_frametime = 0;
			}
			const dmat4 viewMatrix = getCamera()->getViewMatrix();
			const dmat4 viewProjMatrix = getCamera()->getViewProjectionMatrix();

			glStencilFunc(GL_ALWAYS, 1, 0xFF); // Do not test the current value in the stencil buffer, always accept any value on there for drawing
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE); // Make every test succeed

			u32 t = 0;
			_GPUResources.writeGPUMappedData(GPUResources::SSBOSlots::ReadBackData, &t, 1);

			//Compute view frustum planes
			const dmat4 vpt = transpose(viewProjMatrix);
			const std::array frustumPlanes{
				// left, right, bottom, top
				(vpt[3] + vpt[0]),
				(vpt[3] - vpt[0]),
				(vpt[3] + vpt[1]),
				(vpt[3] - vpt[1]),
				// near, far
				(vpt[3] + vpt[2]),
				(vpt[3] - vpt[2]),
			};
			static u32 sentPoints[2];
			u32 pointsToSend = 0;
			u32 prevPointIndex = 0;
			u32 currentPointIndex = 0;
			_meshletCullingShader->setUniform("frustumPlane", frustumPlanes[0], 6);
			_meshletCullingShader->setUniform("windowSize", _appState->_viewportSize);
			sentPoints[_currentBufferId] = 0;
			for (const auto& cloud : _scenes[_activeScene]->_pointClouds | std::views::values) {
				const auto numMeshlets = cloud->getMeshletNumber();
				cloud->updateCloud();
				if (numMeshlets > 0 && cloud->_enabled && cloud->_controlledByRenderer) {
					//auto r = nvtx3::start_range("Meshlet Culling");
					_meshletCullingShader->setUniform("meanMeshletExtent", cloud->_meanMeshletExtent);
					_meshletCullingShader->setUniform("meshletSize", cloud->_meshletSize);
					_meshletCullingShader->setUniform("numMeshlets", (unsigned)numMeshlets);
					_meshletCullingShader->setUniform("modelMatrix", cloud->getModelMatrix());
					dmat4 mvpMatrix = cloud->getMVPMatrix(viewProjMatrix);
					_meshletCullingShader->setUniform("mModelViewProj", mvpMatrix);
					const i32 numGroups = ComputeShader::getNumGroups(numMeshlets);
					const i32 workGroupSize = ComputeShader::getWorkGroupSize(numGroups, numMeshlets);
					_GPUResources.bindBuffer(GPUResources::SSBOSlots::ReadBackData, GPUResources::GPUBinding::ReadBackData);
					cloud->_GPUResources.bindBuffer(GPUResources::SSBOSlots::Meshlets, GPUResources::GPUBinding::Meshlets);
					cloud->_GPUResources.bindBuffer(GPUResources::SSBOSlots::CullingBuffer, GPUResources::GPUBinding::CullingBuffer);

					glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, 8, "Culling");
					_meshletCullingShader->execute(numGroups, workGroupSize);
					glPopDebugGroup();
					cloud->_GPUResources.lockRange(GPUResources::SSBOSlots::CullingBuffer);
					//nvtx3::end_range(r);

					//r = nvtx3::start_range("CompactPointBuffer");
					sentPoints[_currentBufferId] += generateCompactInfo(cloud.get(), prevPointIndex, currentPointIndex, pointsToSend);
					//nvtx3::end_range(r);
				}
			}
			if (pointsToSend) {
				_GPUResources.writeGPUMappedData(GPUResources::SSBOSlots::UploadBuffer, _pointDataToSend.data(), pointsToSend, _currentBufferId * _newPointsBufferSize);
				getCamera()->changed = true;
				//_GPUResources.lockRange(GPUResources::SSBOSlots::RGB);
				//fmt::print("{} points sent to GPU\n", pointsToSend);
			}

			//Reset depth buffer
			_composeImageShader->bind();
			_renderFBO->bindColorAttachment();
			const i32 numGroups = ComputeShader::getNumGroups(_appState->_viewportSize.x * _appState->_viewportSize.y);
			const i32 numWorkGroups = ComputeShader::getWorkGroupSize(numGroups, _appState->_viewportSize.x * _appState->_viewportSize.y);
			_GPUResources.bindBuffer(GPUResources::SSBOSlots::DepthBuffer, GPUResources::GPUBinding::DepthBuffer);
			_resetDepthBuffer->setUniform("windowSize", _appState->_viewportSize);
			_resetDepthBuffer->setUniform("backgroundColor", _appState->_backgroundColor);
			_resetDepthBuffer->execute(numGroups, numWorkGroups);
			const GLsync resetDepthBufferFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

			_GPUResources.readGPUMappedMemory(GPUResources::SSBOSlots::ReadBackData, &_appState->_renderedPoints[_currentBufferId], 1);

			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 2, 14, "BuildBuffer");
			//Compact buffer
			if (sentPoints[_currentBufferId]) {
				const i32 xNumGroups = ComputeShader::getNumGroups(sentPoints[_currentBufferId]);
				const i32 xNumWorkGroups = ComputeShader::getWorkGroupSize(xNumGroups, sentPoints[_currentBufferId]);

				_copyPositionsShader->setUniform("numPointsWithOffset", sentPoints[_currentBufferId]);
				_GPUResources.bindBufferRange(_currentBufferId == 0 ? GPUResources::SSBOSlots::PositionFrame1 : GPUResources::SSBOSlots::PositionFrame2,
				                              GPUResources::GPUBinding::Position,
											  0, _pointsPerSubBuffer * sizeof(vec3));
				_GPUResources.bindBufferRange(_currentBufferId == 0 ? GPUResources::SSBOSlots::AttributeFrame1 : GPUResources::SSBOSlots::AttributeFrame2, 
				                              GPUResources::GPUBinding::Attribute,
											  0, _pointsPerSubBuffer * sizeof(u32));
				_GPUResources.bindBufferRange(GPUResources::SSBOSlots::UploadBuffer, GPUResources::GPUBinding::UploadBuffer, _newPointsBufferSize * _currentBufferId * sizeof(PointWithOffset), _newPointsBufferSize * sizeof(PointWithOffset));
				glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 2, 12, "CopyNewData");
				_copyPositionsShader->execute(xNumGroups, xNumWorkGroups);
				glPopDebugGroup();
			}

			for (const auto& cloud : _scenes[_activeScene]->_pointClouds | std::views::values) {
				const auto numMeshlets = cloud->getMeshletNumber();
				if (numMeshlets > 0 && cloud->_enabled && cloud->isControlledByRenderer()) {
					_GPUResources.bindBufferRange(_currentBufferId == 0 ? GPUResources::SSBOSlots::PositionFrame1 : GPUResources::SSBOSlots::PositionFrame2,
					                              GPUResources::GPUBinding::Position,
												  0, _pointsPerSubBuffer * sizeof(vec3));
					_GPUResources.bindBufferRange(_prevBufferId == 0 ? GPUResources::SSBOSlots::PositionFrame1 : GPUResources::SSBOSlots::PositionFrame2,
					                              GPUResources::GPUBinding::PositionLastFrame,
												  0, _pointsPerSubBuffer * sizeof(vec3));
					_GPUResources.bindBufferRange(_currentBufferId == 0 ? GPUResources::SSBOSlots::AttributeFrame1 : GPUResources::SSBOSlots::AttributeFrame2, 
					                              GPUResources::GPUBinding::Attribute,
												  0, _pointsPerSubBuffer * sizeof(u32));
					_GPUResources.bindBufferRange(_prevBufferId == 0 ? GPUResources::SSBOSlots::AttributeFrame1 : GPUResources::SSBOSlots::AttributeFrame2,
					                              GPUResources::GPUBinding::AttributeLastFrame,
												  0, _pointsPerSubBuffer * sizeof(u32));
					cloud->_GPUResources.bindBuffer(GPUResources::SSBOSlots::CompactionInfo, GPUResources::GPUBinding::CompactionInfo);

					const u32 numInvocations = cloud->_numCompactInfo * cloud->_maxPointsToMove;
					if (numInvocations > 0) {
						const i32 xNumGroups = ComputeShader::getNumGroups(numInvocations);
						const i32 xWorkGroupSize = ComputeShader::getWorkGroupSize(xNumGroups, numInvocations);
						/*_compactPositionsBuffer->setUniform("numMeshlets", (u32)numMeshlets);*/
						_compactPositionsBuffer->setUniform("maxPointsToMove", cloud->_maxPointsToMove);
						_compactPositionsBuffer->setUniform("numCompactInfo", cloud->_numCompactInfo);
						glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 2, 11, "CompactGPU");
						_compactPositionsBuffer->execute(xNumGroups, xWorkGroupSize);
						glPopDebugGroup();
					}
				}
			}
			glPopDebugGroup();

			//_GPUResources.readGPUMappedMemory(GPUResources::SSBOSlots::ReadBackData, &_appState->_renderedPoints[_currentBufferId], 1);
			//Projection and rasterization
			u32 offset = 0;
			for (const auto& cloud : _scenes[_activeScene]->_pointClouds | std::views::values) {
				const auto numMeshlets = cloud->getMeshletNumber();
				if (numMeshlets > 0 && cloud->_enabled) {
					unsigned numPoints = cloud->_numVisiblePoints;
					//fmt::println("Visible points of cloud {}: {}", model.second->_name, numPoints);
					if (numPoints > 0) {
						dmat4 mvpMatrix = cloud->getMVPMatrix(viewProjMatrix);
						_computeDepthBufferShader->setUniform("numPoints", numPoints);
						_computeDepthBufferShader->setUniform("pointsOffset", offset);
						_computeDepthBufferShader->setUniform("mModelViewProj", mvpMatrix);
						_computeDepthBufferShader->setUniform("windowSize", _appState->_viewportSize);
						_GPUResources.bindBufferRange(_currentBufferId == 0 ? GPUResources::SSBOSlots::PositionFrame1 : GPUResources::SSBOSlots::PositionFrame2,
						                              GPUResources::GPUBinding::Position,
													  0, _pointsPerSubBuffer * sizeof(vec3));
						_GPUResources.bindBufferRange(_currentBufferId == 0 ? GPUResources::SSBOSlots::AttributeFrame1 : GPUResources::SSBOSlots::AttributeFrame2, 
						                              GPUResources::GPUBinding::Attribute,
													  0, _pointsPerSubBuffer * sizeof(u32));
						_GPUResources.bindBuffer(GPUResources::SSBOSlots::DepthBuffer, GPUResources::GPUBinding::DepthBuffer);
						i32 xNumGroups = ComputeShader::getNumGroups(numPoints);
						i32 xNumWorkGroups = ComputeShader::getWorkGroupSize(xNumGroups, numPoints);
						glWaitSync(resetDepthBufferFence, 0, GL_TIMEOUT_IGNORED);
						glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 3, 13, "ComputeDepth");
						_computeDepthBufferShader->execute(xNumGroups, xNumWorkGroups);
						glPopDebugGroup();

						xNumGroups = ComputeShader::getNumGroups(_appState->_viewportSize.x * _appState->_viewportSize.y);
						xNumWorkGroups = ComputeShader::getWorkGroupSize(xNumGroups, _appState->_viewportSize.x * _appState->_viewportSize.y);
						_composeImageShader->setUniform("colorOutput", 0);
						vec3 first, second, last;
						/*ImGradient& gradient = _appState->_fusionGradient[PointCloud::_renderAttributes[(u8)cloud->_attToUse]];
						gradient.getColorAt(0, &first[0]);
						gradient.getColorAt(0.5f, &second[0]);
						gradient.getColorAt(1.f, &last[0]);*/
						_composeImageShader->setUniform("firstGradientPoint", first);
						_composeImageShader->setUniform("secondGradientPoint", second);
						_composeImageShader->setUniform("lastGradientPoint", last);
						if (cloud->_attToUse == Attribute::RGB)
							_composeImageShader->setSubroutineUniform(ShaderEnum::COMPUTE_SHADER, ShaderEnum::GET_COLOR, "getColorUnpackingUint");
						else
							_composeImageShader->setSubroutineUniform(ShaderEnum::COMPUTE_SHADER, ShaderEnum::GET_COLOR, "getColorFromFloat");
						_composeImageShader->applyActiveSubroutines();
						glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 4, 13, "ComposeImage");
						_composeImageShader->execute(xNumGroups, xNumWorkGroups);
						glPopDebugGroup();

						if (_appState->_renderWithEDL) {
							_EDLShader->setUniform("zNear", _scenes[_activeScene]->getActiveCamera()->getZnear());
							_EDLShader->setUniform("zFar", _scenes[_activeScene]->getActiveCamera()->getZfar());
							_EDLShader->setUniform("edlStrength", _appState->_edlStrength);
							_EDLShader->setUniform("colorOutput", 0);
							glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 4, 3, "EDL");
							_EDLShader->execute(xNumGroups, xNumWorkGroups);
							glPopDebugGroup();
						}
					}
					offset += numPoints;
				}
			}
			_currentBufferId = (_currentBufferId + 1) % _numBuffers;
			_prevBufferId = (_prevBufferId + 1) % _numBuffers;

			glDeleteSync(resetDepthBufferFence);
			glBlitNamedFramebuffer(_renderFBO->getId(), 0, 0, 0, _renderFBO->getSize().x, _renderFBO->getSize().y, 0, 0, _renderFBO->getSize().x, _renderFBO->getSize().y, GL_COLOR_BUFFER_BIT, GL_NEAREST);


			if (_appState->_renderMeshletsAABBs) {
				for (const auto& cloud : _scenes[_activeScene]->_pointClouds | std::views::values) {
					_meshletsAABBDrawProgram->bind();
					mat4 modelMatrix = cloud->getModelMatrix();
					const auto numMeshlets = cloud->getMeshletNumber();
					_meshletsAABBDrawProgram->setUniform("viewProjMatrix", viewProjMatrix);
					_meshletsAABBDrawProgram->setUniform("modelMatrix", modelMatrix);
					_meshletsAABBDrawProgram->setUniform("meshletSize", cloud->_meshletSize);
					cloud->_GPUResources.bindBuffer(GPUResources::SSBOSlots::Meshlets, GPUResources::GPUBinding::Meshlets);
					cloud->_GPUResources.bindBuffer(GPUResources::SSBOSlots::CullingBuffer, GPUResources::GPUBinding::CullingBuffer);
					//_meshletsAABBDrawProgram->setUniform("numMeshlets", numMeshlets);
					glDrawArrays(GL_LINES, 0, numMeshlets * 24);
				}
			}

			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // Make sure you will no longer (over)write stencil values, even if any test succeeds
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Make sure we draw on the backbuffer again.

			glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // Now we will only draw pixels where the corresponding stencil buffer value equals 1

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);					// Back to initial state
			glDepthFunc(GL_LESS);

			//Grid draw
			if (_appState->_renderGrid) {
				_gridShaderProgram->bind();
				_gridShaderProgram->setUniform("mModelViewProj", viewProjMatrix);
				_gridShaderProgram->setUniform("gridColor", _appState->_gridColor);
				glBindVertexArray(_gridVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		} catch (std::exception& e) {
			spdlog::error("Error while rendering: {}", e.what());
		}

		glPopDebugGroup();
	}
}


//==========================
//			Scene
//==========================

// - Adders

/**
 * \brief Inserts 3D model to current scene
 *
 *
 */
unsigned Nimbus::Renderer::addModel(Model3D* model) {
	i32 dupeCount = 0;
	std::string idAppend;
	if (_scenes.empty())
		this->newScene();
	this->getCamera()->lookModel(model);
	if (const auto pointCloud = dynamic_cast<PointCloud*>(model)) {
		while (_scenes[_activeScene]->_pointClouds.contains(pointCloud->getName() + idAppend)) {
			dupeCount++;
			idAppend = " (" + std::to_string(dupeCount) + ")";
		}
		pointCloud->_name += idAppend;

		_scenes[_activeScene]->_pointClouds.insert(std::make_pair(pointCloud->getName(), std::unique_ptr<PointCloud>(pointCloud)));

		getCamera()->changed = true;
		return _scenes[_activeScene]->_pointClouds.size() - 1;
	}
	return UINT32_MAX;
}

/**
 * \brief Inserts new simple light to current scene
 *
 * \returns index of new light in the scene collection
 */
unsigned Nimbus::Renderer::addLight() const {
	_scenes[_activeScene]->_light.push_back(std::make_unique<Light>());
	return _scenes[_activeScene]->_light.size() - 1;
}

/**
 * \brief Inserts new simple camera to current scene
 *
 * \returns index of new camera in the scene collection
 */
unsigned Nimbus::Renderer::addCamera() const {
	_scenes[_activeScene]->_camera.push_back(std::make_unique<Camera>());
	return _scenes[_activeScene]->_camera.size() - 1;
}

// - Deleters

void Nimbus::Renderer::deletePointCloud(const std::string& cloudName) const {
	_scenes[_activeScene]->_pointClouds.erase(cloudName);
	//_gui->resetPicking();
}

void Nimbus::Renderer::deleteLight(const unsigned lightIdx) const {
	_scenes[_activeScene]->_light[lightIdx].reset();
	for (size_t i = lightIdx; i < _scenes[_activeScene]->_light.size() - 1; i++) {
		_scenes[_activeScene]->_light[i].swap(_scenes[_activeScene]->_light[i + 1]);
	}
	_scenes[_activeScene]->_light.pop_back();
}

void Nimbus::Renderer::deleteCamera(const unsigned cameraIdx) const {
	const auto scene = _scenes[_activeScene].get();
	scene->_camera[cameraIdx].reset();
	for (size_t i = cameraIdx; i < scene->_camera.size() - 1; i++) {
		scene->_camera[i].swap(scene->_camera[i + 1]);
	}
	scene->_camera.pop_back();
	scene->_activeCamera = (scene->_activeCamera + 1) % scene->_camera.size();
}

/**
 * \brief Deletes active scene. Makes new empty scene if no active scenes left
 *
 * \return true if new empty scene was made; false otherwise
 */
bool Nimbus::Renderer::deleteScene() {
	bool newSceneMade = false;
	_scenes[_activeScene].reset();
	for (size_t i = _activeScene; i < _scenes.size() - 1; i++) {
		_scenes[i].swap(_scenes[i + 1]);
	}
	_scenes.pop_back();
	if (_scenes.empty()) {
		newScene();
		newSceneMade = true;
	} else {
		_activeScene %= _scenes.size();
	}

	return newSceneMade;
}

// - Getters

/**
 * \brief get the cloud with the cloud name specified if it exists in the current active scene.
 * \param cloudName to search for.
 * \return pointer to the cloud if found, nullptr otherwise.
 */
Nimbus::PointCloud* Nimbus::Renderer::getPointCloud(const std::string& cloudName) const {
	const auto& cloud = _scenes[_activeScene]->_pointClouds.find(cloudName);
	if (cloud != _scenes[_activeScene]->_pointClouds.end())
		return cloud->second.get();
	return nullptr;
}

//- Others

Nimbus::FBORender* Nimbus::Renderer::getRenderFBO() const {
	return _renderFBO.get();
}

/**
 * \brief Changes the active Scene
 * \param newActiveScene new active scene.
 * \throw std::out_of_range in case of trying to change to a scene that doesn't exist.
 */
void Nimbus::Renderer::changeActiveScene(const unsigned& newActiveScene) {
	if (newActiveScene >= _scenes.size())
		throw std::out_of_range("[Nimbus::Renderer::changeActiveScene] -> You can't activate a scene that doesn't exist.");
	_activeScene = newActiveScene;
}

Nimbus::Scene* Nimbus::Renderer::newScene() {
	_scenes.push_back(std::make_unique<Scene>(_appState->_viewportSize));
	_activeScene = _scenes.size() - 1;

	return _scenes[_activeScene].get();
}

void Nimbus::Renderer::saveScene(const std::string& filepath, const std::string& filename) const {
	_scenes[_activeScene]->save(filepath, filename);
}

void Nimbus::Renderer::loadScene(const std::string& filepath, const std::string& filename) {
	newScene();
	_scenes[_activeScene]->_camera.clear();
	_scenes[_activeScene]->_light.clear();
	_scenes[_activeScene]->load(filepath, filename);
}

void Nimbus::Renderer::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename,
									 const efsw::Action action, std::string oldFilename) {
	switch (action) {
	case efsw::Actions::Add:
		spdlog::info("File {} has been added to {}", filename, dir);
		break;
	case efsw::Actions::Delete:
		spdlog::info("File {} has been deleted from {}", filename, dir);
		break;
	case efsw::Actions::Modified:
		spdlog::info("File {} has been modified in {}", filename, dir);
		_needToReloadComputeShaders = true;
		break;
	case efsw::Actions::Moved:
		spdlog::info("File {} has been renamed to {}", oldFilename, filename);
		break;
	default:
		spdlog::error("Should never happen!");
	}
}

