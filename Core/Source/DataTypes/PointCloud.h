#pragma once

#include <array>
#include <vector>
#include <glm/fwd.hpp>

#include "Model3D.h"
#include "Graphics/GPUResources.h"
#include "Graphics/VAO.h"

namespace Nimbus {

	enum class Attribute : u8 {
		RGB,
		NumAttributes
	};

	class PointCloud : public Model3D {
		friend class DetailsWindow;
		friend class PickingWindow;
		friend class Renderer;
		friend class MemoryManager;
		friend class FileManager;
	public:
		enum class SortingMethod {
			Hilbert,
			Morton,
			kDTree
		};

		struct CompactInfo {
			u32 prevIndex;
			u32 newIndex;
			u16 length;
		};

		union u32Bits {
			f32 asFloat;
			u32 asUint;
		};

		struct Meshlet {
			u16 pointsLoaded;
			u16 pointsRequired;
			AABB aabb;

			std::mutex mutex;

			std::vector<vec3> pos;
			std::vector<u32Bits> att;

			Meshlet() : pointsLoaded(0), pointsRequired(0) {
			}

			Meshlet(const Meshlet& other) : pointsLoaded(other.pointsLoaded), pointsRequired(other.pointsRequired),
				aabb(other.aabb), pos(other.pos), att(other.att) {

			}
		};

		inline static u16 maxBandsPerAtt[(u8)Attribute::NumAttributes]{
			1,      //RGB
		};

		inline static std::array _renderAttributes{
			"RGB",
		};

	protected:
		//Data
		std::vector<vec3>																_position;
		std::vector<u32>																_rgbColor;
		std::vector<Meshlet>															_meshlets;
		std::vector<vec3>																_normals;
		std::array<std::vector<std::vector<float>>, (u8)Attribute::NumAttributes>		_fusionData; //Data source - band - data
		std::array<std::vector<vec2>, (u8)Attribute::NumAttributes>						_fusionDataMinMax;
		std::array<u16, (u8)Attribute::NumAttributes>									_availableDataBands;
		std::vector<std::unique_ptr<PointCloud>>										_subClouds;

		//This bools are mostly used to mark for update rendering resources.
		bool																_meshletsNeedUpdate = false;
		bool																_optimized = false;
		bool																_controlledByRenderer = true;

		//Render things
		GPUResources														_GPUResources;
		std::vector<u16>													_currentFrameCulling;
		std::vector<u16>													_lastFrameCulling;
		i32																	_attributeBand = 0;
		i32																	_prevAttributeBand = 0;
		Attribute															_attToUse = Attribute::RGB;
		Attribute															_prevAttToUse = Attribute::RGB;
		u32																	_meshletSize = 1024;
		float																_meanMeshletExtent;
		u32																	_numVisiblePoints;
		u32																	_numCompactInfo;
		u32																	_maxPointsToMove;

		//IO things
		std::string															_savePath;
		std::vector<std::ifstream>											_posRf;
		std::vector<std::ifstream>											_attRf;
		u64																	_numPoints;
		u64																	_currentAttFileSize;

		//Picking things
		std::vector<u8>														_picked;
		bool																_pickedNeedUpdate = true;

		bool																_loading = true;

		void reloadAttribute();
		void updateCloud();

		//Internal IO functions
		void initializeReadStreams();
		void reopenAttributeReadStream(std::ifstream& rf);
		void closeReadStreams();
		void saveMetadata(const std::string& path);
		void savePosition(std::ofstream& wf);
		void saveRGB(std::ofstream& wf);
		void saveFusionData(std::ofstream& wf, Attribute dataSource, u32 band);
		void loadMeshletPosition(std::ifstream& rf, u32 meshletIndex, u16 numPointsToLoad, u16 insideMeshletOffset);
		void loadMeshletAttribute(std::ifstream& rf, u32 meshletIndex, u16 numPointsToLoad, u16 insideMeshletOffset);
		void loadPositionsFromBinaryFile(const std::string& path);
		void loadRGBFromBinaryFile(const std::string& path);

		//Meshlet generation
		static u32 hilbert3D(const vec3& point, const AABB& aabb);
		std::vector<u32> hilbertSorting() const;
		std::vector<u32> mortonSorting() const;
		std::vector<u32> kdTreeSorting();
		void computeMetrics(const std::string& methodName);
		static u32 computeMeshletSize(u64 numPoints);
		void recomputeMeshlets();

	public:
		explicit PointCloud(const std::string& name);

		PointCloud(const std::string& name, const AABB& aabb, const vec3& pos = { 0,0,0 }, const vec3& rot = { 0,0,0 }, const vec3& scale = { 1,1,1 });

		PointCloud(const std::string& name, const std::vector<VAO::Point>& points, const AABB& aabb, const vec3& pos = { 0,0,0 }, const vec3& rot = { 0,0,0 }, const vec3& scale = { 1,1,1 });

		virtual ~PointCloud();

		void spatialOrdering(SortingMethod sortMethod = SortingMethod::Hilbert, bool saveMeshlets = true, bool shuffleMeshlets = true, const bool clearBuffers = true);

		void drawModel(dmat4 viewProjMatrix) override;
		void needUpdate();

		void addSubcloud(std::unique_ptr<PointCloud> newSubcloud);
		void addNewPoint(const VAO::Point& newPoint);
		void addNewPoints(const std::vector<VAO::Point>& newPoints);
		void addNewPoints(glm::vec3* points, glm::u32* rgb, size_t offset, size_t size, const AABB& aabb);
		void addNormals(const std::vector<vec3>& normals);

		void setNewPoints(const std::vector<VAO::Point>& newPoints, bool optimizePointOrder = false);
		void setNormals(const std::vector<vec3>& newNormals);
		void setColor(const std::vector<vec3>& colorVector);
		void setColor(const std::vector<vec4>& colorVector);
		void setData(const Attribute& att, u32 band, const std::vector<float>& dataVector);

		void clearData();

		PointCloud* getSubcloud(const std::string& subcloudName) const;
		const std::vector<std::unique_ptr<PointCloud>>& getSubclouds();
		std::vector<vec3>& getNormals();
		std::vector<vec3> getPointsPosition();
		std::vector<u32> getPointsColor();
		const std::vector<Meshlet>& getMeshlets();
		u32 getMeshletSize() const;
		size_t getMeshletNumber() const;
		u64 getNumberOfPoints() const;
		std::string getCloudSummary() const;
		bool isEnabled() const;
		bool isControlledByRenderer() const;

		template<Attribute Att>
		bool have(u16 band = 0) const;
		bool haveNormals() const;

		void saveBinaryFile(const std::string& path) override;
		void loadBinaryFile(const std::string& path) override;

		static std::string AttributeToString(const Attribute& dataSource);

		void updateAABB(const AABB& aabb);
		void updateAABB(const vec3& point);

		void switchAttribute(const Attribute& att, const u16 & = 0);
		void requestMeshletData(u32 meshletIndex, u16 numData = 0);
		void unloadMeshletData(u32 meshletIndex);
		void returnControlToRenderer();
		vec3 getPosition(u32 pointIndex);
		std::vector<vec3> getPositions(u32 offset, u32 numPoints) const;

		template <Attribute Att>
		u32Bits getAttribute(u32 pointIndex, u16 band = 0);

		template<Attribute Att>
		std::vector<u32Bits> getAttribute(u32 offset, u32 numPoints, u16 band) const;

		template<Attribute Att>
		vec2 getAttributeMinMax(u16 band) const;
		Attribute getCurrentAttribute() const;
	};

	template <Attribute Att>
	bool PointCloud::have(const u16 band) const {
		return _availableDataBands[(u8)Att] > band;
	}

	template <Attribute Att>
	PointCloud::u32Bits PointCloud::getAttribute(const u32 pointIndex, const u16 band) {
		_controlledByRenderer = false;
		switchAttribute(Att, band);
		u32 meshletIndex = pointIndex / _meshletSize;
		u16 pointInsideMeshlet = pointIndex % _meshletSize;
		requestMeshletData(meshletIndex, _meshletSize);
		while (_meshlets[meshletIndex].pointsLoaded <= pointInsideMeshlet) {
			std::this_thread::yield();
		}
		return _meshlets[meshletIndex].att[pointInsideMeshlet];
	}

	template <Attribute Att>
	std::vector<PointCloud::u32Bits> PointCloud::getAttribute(const u32 offset, const u32 numPoints, const u16 band) const {
		std::ifstream rf;
		std::string fileExtension = std::format(".NimbusCloud{}", std::string(_renderAttributes[(u8)Att]).substr(0, 3));
		rf.open(_savePath + fileExtension, std::ios::in | std::ios::binary);
		std::vector<u32Bits> data(numPoints);
		if (rf.is_open()) {
			rf.seekg(offset * sizeof(u32) + band * _numPoints * sizeof(u32));
			rf.read((char*)data.data(), numPoints * sizeof(u32));
			//if (rf.fail())
			//	spdlog::error("Error reading attribute data from file [Band {}]", band);
		}
		return std::move(data);
	}

	template <Attribute Att>
	vec2 PointCloud::getAttributeMinMax(u16 band) const {
		return _fusionDataMinMax[(u8)Att][band];
	}

}
