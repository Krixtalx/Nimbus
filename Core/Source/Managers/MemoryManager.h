#pragma once
#include "ConcurrentQueue/concurrentqueue.h"

namespace Nimbus {
	class PointCloud;
}

namespace Nimbus {
	class MemoryManager {
	public:
		struct LoadMeshletTask {
			PointCloud* pointCloud;
			u32 meshletIndex;
			u8 att;
			u16 band;
		};

		static constexpr uint8_t _numThreads = 8;
	private:
		inline static bool keepRunning = false;
		static constexpr float	 _limit = 0.85f;
		inline static uint64_t	 _currentRamUsage = 0;
		inline static uint64_t	 _maxRam = 0;
		inline static uint64_t	 _currentVramUsage = 0;
		inline static uint64_t	 _maxVram = 0;
		inline static moodycamel::ConcurrentQueue<LoadMeshletTask> _loadTasks;// = moodycamel::ConcurrentQueue<LoadMeshletTask>(65536);
		inline static std::array<std::thread, _numThreads> _loadThreads;

	public:
		static void updateCurrentRamUsage();
		static float currentRamUsage();
		static float currentVramUsage();
		static bool mustFreeMemory();
		static void generateLoadTask(PointCloud* pc);
		static void spawnLoadThreads();
		static void shutdownLoadThreads();
	};
}
