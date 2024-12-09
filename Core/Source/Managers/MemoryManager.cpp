#include "CorePch.h"
#include "MemoryManager.h"

#include "psapi.h"
#include "glad/glad.h"								

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049


void Nimbus::MemoryManager::updateCurrentRamUsage() {
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	_maxRam = memInfo.ullTotalPhys; //(u64)1073741824 * (u64)4;
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc));
	_currentRamUsage = pmc.WorkingSetSize;

	int32_t temp = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &temp);
	_maxVram = temp;
	glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &temp);
	_currentVramUsage = _maxVram - temp;
}

/**
 * \brief Returns current RAM usage in MBs
 * \return RAM usage in MBs
 */
float Nimbus::MemoryManager::currentRamUsage() {
	return static_cast<float>(_currentRamUsage) / 1024.f / 1024.f;
}

/**
 * \brief Returns current Vram usage in MBs
 * \return Vram usage in MBs
 */
float Nimbus::MemoryManager::currentVramUsage() {
	return static_cast<float>(_currentVramUsage) / 1024.f;
}

bool Nimbus::MemoryManager::mustFreeMemory() {
	return static_cast<float>(_currentRamUsage) / _maxRam > _limit;
}

void Nimbus::MemoryManager::generateLoadTask(PointCloud* pc) {
	static u32 meshletIndex;
	meshletIndex = 0;
	for (const auto& meshlet : pc->_meshlets) {
		if ((mustFreeMemory() && meshlet.pointsRequired < meshlet.pointsLoaded) || (meshlet.pointsRequired > meshlet.pointsLoaded)) {
			_loadTasks.try_enqueue({ pc, meshletIndex, (u8)pc->_attToUse, (u16)pc->_attributeBand });
		}
		meshletIndex++;
	}
}


void Nimbus::MemoryManager::spawnLoadThreads() {
	keepRunning = true;
	for (int id = 0; id < _numThreads; ++id) {
		std::thread t([id] {
			LoadMeshletTask task;
			while (keepRunning) {
				while (_loadTasks.try_dequeue(task)) {
					if ((u8)task.pointCloud->_attToUse == task.att && task.pointCloud->_attributeBand == task.band) {
						PointCloud* pc = task.pointCloud;
						const u32 mshId = task.meshletIndex;
						PointCloud::Meshlet& meshlet = pc->_meshlets[mshId];

						if (meshlet.mutex.try_lock()) {
							u32 pointsRequired = meshlet.pointsRequired;
							const bool needToAllocate = pointsRequired > meshlet.pos.size();
							const bool needToFreeMemory = mustFreeMemory();
							if (needToFreeMemory) {
								meshlet.pointsLoaded = std::min(pointsRequired, (u32)meshlet.pointsLoaded);
								meshlet.pos.resize(pointsRequired);
								meshlet.att.resize(pointsRequired);
								meshlet.pos.shrink_to_fit();
								meshlet.att.shrink_to_fit();
							} else if (needToAllocate) {
								pointsRequired *= 1.25f;
								pointsRequired = std::clamp(pointsRequired, (u32)0, pc->_meshletSize);
								meshlet.pos.resize(pointsRequired);
								meshlet.att.resize(pointsRequired);
							}

							if (meshlet.pos.size() > meshlet.pointsLoaded) {
								const uint16_t pointsToLoad = meshlet.pos.size() - meshlet.pointsLoaded;
								pc->loadMeshletPosition(pc->_posRf[id], mshId, pointsToLoad, meshlet.pointsLoaded);
								pc->loadMeshletAttribute(pc->_attRf[id], mshId, pointsToLoad, meshlet.pointsLoaded);

								meshlet.pointsLoaded = meshlet.pos.size();
							} else if (meshlet.pos.size() < meshlet.pointsLoaded) {
								spdlog::error("Esto no deberia estar pasando");
								meshlet.pointsLoaded = 0;
							}
							meshlet.mutex.unlock();
						}
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		});
		_loadThreads[id] = std::move(t);
	}
}

void Nimbus::MemoryManager::shutdownLoadThreads() {
	keepRunning = false;
	for (int id = 0; id < _numThreads; ++id) {
		_loadThreads[id].join();
	}
}
