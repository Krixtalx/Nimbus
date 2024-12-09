#pragma once
#include "glad/glad.h"								// include glad to get all the required OpenGL headers
#include <spdlog/spdlog.h>

namespace Nimbus {
	struct BufferRange {
		size_t startOffset;
		size_t length;

		bool overlaps(const BufferRange& _rhs) const {
			return startOffset < (_rhs.startOffset + _rhs.length)
				&& _rhs.startOffset < (startOffset + length);
		}
	};

	struct BufferLock {
		BufferRange range;
		GLsync syncObj;
	};

	struct MappedMemory {
		void* gpuMemoryPtr = nullptr;
		size_t dataSize = 0;

		MappedMemory() = default;
		MappedMemory(MappedMemory& other) = default;

		template<typename T>
		void set(const uint32_t index, T val) const {
			set(index, &val, 1);
		}

		template<typename T>
		void set(const uint32_t index, T* val, const size_t length) {
			if (gpuMemoryPtr) {
				if (dataSize != sizeof(T))
					spdlog::warn("Mismatch betweet mappedMemory data type and val data type");
				void* ptr = ((unsigned char*)gpuMemoryPtr + dataSize * index);
				memcpy(ptr, val, sizeof(T) * length);
				//lockRange(dataSize * index, sizeof(T) * length);
			} else {
				spdlog::warn("Trying to set into empty mapped memory");
			}
		}

		void* operator[](const uint32_t index) const {
			return ((unsigned char*)gpuMemoryPtr + dataSize * index);
		}

		void operator=(const MappedMemory& other) {
			if (this != &other) {
				gpuMemoryPtr = other.gpuMemoryPtr;
				dataSize = other.dataSize;
			}
		}

		/**
		 * \brief Locks code execution until the range fence is signalled.
		 * \param lockBeginBytes Beginning of the locked range.
		 * \param lockLength How much bytes will the fence lock.
		 * \param waitOnCpu If the CPU should wait to the fence to be signalled. Otherwise, the GPU will wait for the CPU to signal the fence.
		 */
		void waitForLockedRange(const size_t lockBeginBytes, const size_t lockLength, const bool waitOnCpu) {
			const BufferRange testRange = { lockBeginBytes, lockLength };
			std::vector<BufferLock> swapLocks;
			for (auto& bufferLock : bufferLocks) {
				if (testRange.overlaps(bufferLock.range)) {
					wait(&bufferLock.syncObj, waitOnCpu);
					cleanup(&bufferLock);
				} else {
					swapLocks.push_back(bufferLock);
				}
			}

			bufferLocks.swap(swapLocks);
		}

		/**
		 * \brief Add a fence for a certain mapped memory range. The lock range is only used to know if the waitForLockedRange should lock or not.
		 * \param lockBeginBytes Beginning of the locked range.
		 * \param lockLength How much bytes will the fence lock.
		 */
		void lockRange(const size_t lockBeginBytes, const size_t lockLength) {
			const BufferRange newRange = { lockBeginBytes, lockLength };
			const GLsync syncName = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			const BufferLock newLock = { newRange, syncName };

			bufferLocks.push_back(newLock);
		}

	private:
		std::vector<BufferLock> bufferLocks;

		static void wait(const GLsync* syncObj, const bool waitOnCPU) {
			if (waitOnCPU) {
				GLbitfield waitFlags = 0;
				GLuint64 waitDuration = 0;
				bool keepGoing = true;
				while (keepGoing) {
					const GLenum waitRet = glClientWaitSync(*syncObj, waitFlags, waitDuration);

					if (waitRet == GL_WAIT_FAILED) {
						spdlog::warn("Buffer lock wait failed for some reason");
						return;
					}

					keepGoing = !(waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED);

					// After the first time, need to start flushing, and wait for a looong time.
					waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
					waitDuration = 100000000;
				}
			} else {
				glWaitSync(*syncObj, 0, GL_TIMEOUT_IGNORED);
			}
		}

		static void cleanup(const BufferLock* bufferLock) {
			glDeleteSync(bufferLock->syncObj);
		}
	};

	struct GPUResources {
		enum class GPUBinding : uint8_t {
			DepthBuffer,
			CullingBuffer,
			CompactionInfo,
			UploadBuffer,
			Meshlets,
			ReadBackData,
			PickedPoint,
			Position,
			PositionLastFrame,
			Attribute,
			AttributeLastFrame,
			//ND_VegetationIndex, R_VegetationIndex, GR_VegetationIndex, NDRE_VegetationIndex,
			Total
		};
		enum class SSBOSlots : uint8_t {
			DepthBuffer,
			CullingBuffer,
			CompactionInfo,
			UploadBuffer,
			Meshlets,
			ReadBackData,
			PickedPoint,
			Position,
			Attribute,
			//ND_VegetationIndex, R_VegetationIndex, GR_VegetationIndex, NDRE_VegetationIndex,
			Total
		};

		inline static size_t totalUsedMemory = 0;
		inline static GLint64 maxMemoryPerSSBO = 0;
		std::array<size_t, (uint8_t)SSBOSlots::Total>	_memoryReserved;
		std::array<GLuint, (uint8_t)SSBOSlots::Total>	_ssboIds;
		std::array<MappedMemory, (uint8_t)SSBOSlots::Total>	_mappedMemory;

		GPUResources() {
			for (auto& ssboId : _ssboIds) {
				ssboId = 0;
			}
			for (auto& memoryReserved : _memoryReserved) {
				memoryReserved = 0;
			}
		}

		~GPUResources() {
			clearResources();
		}
		//MappedMemory getMappedBuffer(const SSBOSlots ssboSlot) const;
		void bindBuffer(SSBOSlots ssboSlot, GPUBinding gpuBindingSlot) const;
		void bindBufferRange(SSBOSlots ssboSlot, GPUBinding gpuBindingSlot, size_t offsetInBytes, size_t sizeInBytes) const;
		void lockRange(SSBOSlots ssboSlot, const size_t lockBeginBytes = 0, const size_t lockLength = UINT64_MAX);
		void waitForLockedRange(SSBOSlots ssboSlot, bool waitOnCPU = true, const size_t lockBeginBytes = 0, const size_t lockLength = UINT64_MAX);
		void swapBuffers(SSBOSlots originSsboSlot, SSBOSlots destSsboSlot);
		void clearResource(SSBOSlots ssboSlot);
		void clearResources();


		template<typename T>
		MappedMemory reserveGPUMappedMemory(const SSBOSlots ssboSlot, const size_t dataCount, const GLbitfield readOrWriteFlags) {
			const GLbitfield mapFlags = readOrWriteFlags | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			reserveGPUMemory<T>(ssboSlot, dataCount, mapFlags | GL_DYNAMIC_STORAGE_BIT);
			const auto ptr = glMapNamedBufferRange(_ssboIds[(uint8_t)ssboSlot], 0, dataCount * sizeof(T), mapFlags);
			_mappedMemory[(uint8_t)ssboSlot].gpuMemoryPtr = ptr;
			_mappedMemory[(uint8_t)ssboSlot].dataSize = sizeof(T);
			return _mappedMemory[(uint8_t)ssboSlot];
		}

		template<typename T>
		void reserveGPUMemory(const SSBOSlots ssboSlot, const size_t dataCount, const GLbitfield memFlags = GL_DYNAMIC_STORAGE_BIT) {
			const size_t bufferSize = dataCount * sizeof(T);
			if (bufferSize <= maxMemoryPerSSBO) {
				if (_ssboIds[(uint8_t)ssboSlot] != 0) {
					clearResource(ssboSlot);
				}
				glCreateBuffers(1, &_ssboIds[(uint8_t)ssboSlot]);
				glNamedBufferStorage(_ssboIds[(uint8_t)ssboSlot], bufferSize, nullptr, memFlags);
				_memoryReserved[(uint8_t)ssboSlot] = bufferSize;
				totalUsedMemory += _memoryReserved[(uint8_t)ssboSlot];
			} else {
				spdlog::warn("Trying to reserve more memory than permited by MAX_SHADER_STORAGE_BLOCK_SIZE");
			}
		}

		template<typename T>
		void uploadToGPU(const SSBOSlots ssboSlot, T* data, const size_t dataCount, const size_t offset = 0, const GLbitfield memFlags = GL_DYNAMIC_STORAGE_BIT) {
			const size_t bufferSize = dataCount * sizeof(T);
			if (dataCount != 0) {
				if (bufferSize <= maxMemoryPerSSBO) {
					if (bufferSize > _memoryReserved[(uint8_t)ssboSlot]) {
						if (_ssboIds[(uint8_t)ssboSlot] != 0) {
							clearResource(ssboSlot);
						}
						glCreateBuffers(1, &_ssboIds[(uint8_t)ssboSlot]);
						glNamedBufferStorage(_ssboIds[(uint8_t)ssboSlot], bufferSize, data, memFlags);
						_memoryReserved[(uint8_t)ssboSlot] = bufferSize;
						totalUsedMemory += _memoryReserved[(uint8_t)ssboSlot];
					} else {
						glNamedBufferSubData(_ssboIds[(uint8_t)ssboSlot], offset, bufferSize, data);
					}
				} else {
					spdlog::warn("Trying to reserve more memory than permited by MAX_SHADER_STORAGE_BLOCK_SIZE");
				}
			} else {
				clearResource(ssboSlot);
				spdlog::warn("Didn't upload any data to GPU because data buffer is empty. Clearing resource instead");
			}
		}

		template<typename T>
		void writeGPUData(const SSBOSlots ssboSlot, T* data, const size_t dataCount, const size_t offset = 0) const {
			const size_t bufferSize = dataCount * sizeof(T);
			if (bufferSize <= _memoryReserved[(uint8_t)ssboSlot]) {
				glNamedBufferSubData(_ssboIds[(uint8_t)ssboSlot], offset, bufferSize, data);
			} else {
				spdlog::warn("Trying to write more GPU memory than reserved");
			}
		}

		template<typename T>
		void writeGPUMappedData(const SSBOSlots ssboSlot, T* data, const size_t dataCount, const size_t offset = 0) {
			const size_t bufferSize = dataCount * sizeof(T);
			if (bufferSize <= _memoryReserved[(uint8_t)ssboSlot]) {
				if (_mappedMemory[(uint8_t)ssboSlot].gpuMemoryPtr) {
					_mappedMemory[(uint8_t)ssboSlot].set(offset, data, dataCount);
					const size_t offsetSize = offset * sizeof(T);
					//_mappedMemory[(uint8_t)ssboSlot].lockRange(offsetSize, bufferSize);
				} else {
					spdlog::warn("Trying to write GPU memory as mapped memory when it isn't");
				}
			} else {
				spdlog::warn("Trying to write more GPU memory than reserved");
			}
		}

		template<typename T>
		void readGPUMemory(const SSBOSlots ssboSlot, T* data, const size_t dataCount, const size_t offset = 0) const {
			const size_t offsetSize = offset * sizeof(T);
			const size_t bufferSize = dataCount * sizeof(T);
			glGetNamedBufferSubData(_ssboIds[(unsigned)ssboSlot], offsetSize, bufferSize, data);
		}

		template<typename T>
		void readGPUMappedMemory(const SSBOSlots ssboSlot, T* data, const size_t dataCount, const size_t offset = 0) {
			if (_mappedMemory[(uint8_t)ssboSlot].gpuMemoryPtr) {
				const size_t bufferSize = dataCount * sizeof(T);
				const T* src = (T*)_mappedMemory[(uint8_t)ssboSlot][offset];
				memcpy(data, src, bufferSize);
			} else {
				spdlog::warn("Trying to read GPU memory as mapped memory when it isn't");
			}
		}
	};

	/*inline MappedMemory GPUResources::getMappedBuffer(const SSBOSlots ssboSlot) const {
		return _mappedMemory[(uint8_t)ssboSlot];
	}*/

	inline void GPUResources::bindBuffer(const SSBOSlots ssboSlot, const GPUBinding gpuBindingSlot) const {
		if (_ssboIds[(uint8_t)ssboSlot] == 0)
			spdlog::warn("Binding of null SSBO: {}", (uint8_t)ssboSlot);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, (uint8_t)gpuBindingSlot, _ssboIds[(uint8_t)ssboSlot]);
	}

	inline void GPUResources::bindBufferRange(SSBOSlots ssboSlot, GPUBinding gpuBindingSlot, const size_t offsetInBytes,
											  const size_t sizeInBytes) const {
		if (_ssboIds[(uint8_t)ssboSlot] == 0)
			spdlog::warn("Binding of null SSBO: {}", (uint8_t)ssboSlot);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, (uint8_t)gpuBindingSlot, _ssboIds[(uint8_t)ssboSlot], offsetInBytes, sizeInBytes);
	}

	inline void GPUResources::lockRange(SSBOSlots ssboSlot, const size_t lockBeginBytes, const size_t lockLength) {
		auto& mappedMemory = _mappedMemory[(uint8_t)ssboSlot];
		if (mappedMemory.gpuMemoryPtr) {
			mappedMemory.lockRange(lockBeginBytes, lockLength);
		} else {
			spdlog::warn("You can only lock range in mapped memory. Trying to lock {}", (uint8_t)ssboSlot);
		}
	}

	inline void GPUResources::waitForLockedRange(SSBOSlots ssboSlot, const bool waitOnCPU,
												 const size_t lockBeginBytes, const size_t lockLength) {
		auto& mappedMemory = _mappedMemory[(uint8_t)ssboSlot];
		if (mappedMemory.gpuMemoryPtr) {
			mappedMemory.waitForLockedRange(lockBeginBytes, lockLength, waitOnCPU);
		} else {
			spdlog::warn("You can only wait for locked range in mapped memory. Trying to wait in {}", (uint8_t)ssboSlot);
		}
	}

	inline void GPUResources::swapBuffers(SSBOSlots originSsboSlot, SSBOSlots destSsboSlot) {
		const uint32_t temp = _ssboIds[(uint8_t)originSsboSlot];
		_ssboIds[(uint8_t)originSsboSlot] = _ssboIds[(uint8_t)destSsboSlot];
		_ssboIds[(uint8_t)destSsboSlot] = temp;

		if (_mappedMemory[(uint8_t)originSsboSlot].gpuMemoryPtr) {
			const auto temp2 = _mappedMemory[(uint8_t)originSsboSlot];
			_mappedMemory[(uint8_t)originSsboSlot] = _mappedMemory[(uint8_t)destSsboSlot];
			_mappedMemory[(uint8_t)destSsboSlot] = temp2;
		}
	}

	inline void GPUResources::clearResource(const SSBOSlots ssboSlot) {
		if (_mappedMemory[(uint8_t)ssboSlot].gpuMemoryPtr) {
			glUnmapNamedBuffer(_ssboIds[(uint8_t)ssboSlot]);
			_mappedMemory[(uint8_t)ssboSlot].gpuMemoryPtr = nullptr;
			_mappedMemory[(uint8_t)ssboSlot].dataSize = 0;
		}
		if (_memoryReserved[(uint8_t)ssboSlot] > 0) {
			glDeleteBuffers(1, &_ssboIds[(uint8_t)ssboSlot]);
			_ssboIds[(uint8_t)ssboSlot] = 0;
			totalUsedMemory -= _memoryReserved[(uint8_t)ssboSlot];
			_memoryReserved[(uint8_t)ssboSlot] = 0;
		}
	}

	inline void GPUResources::clearResources() {
		for (uint8_t ssboSlot = 0; ssboSlot < (uint8_t)SSBOSlots::Total; ++ssboSlot) {
			clearResource(static_cast<SSBOSlots>(ssboSlot));
		}
	}
}