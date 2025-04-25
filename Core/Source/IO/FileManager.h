#pragma once

namespace Nimbus {
	class PointCloud;

	class FileManager {
		struct LasHeader {
			int			_versionMajor = 0;
			int			_versionMinor = 0;
			std::string	_name;
			std::string	_filePath;
			uint64_t	_headerSize = 0;
			uint64_t	_offsetToPointData = 0;
			uint64_t	_format = 0;
			uint64_t	_bytesPerPoint = 0;
			uint64_t	_numPoints = 0;
			dvec3		_scale = { 0.0, 0.0, 0.0 };
			dvec3		_offset = { 0.0, 0.0, 0.0 };
			dvec3		_min = { 0.0, 0.0, 0.0 };
			dvec3		_max = { 0.0, 0.0, 0.0 };
		};

		struct ReadTask
		{
			LasHeader* _header;
			std::string _name, _rootPath;
			i64			_offset;
			i64			_length;
		};

		static constexpr u64 pointsPerBatch = 20000000;

		static glm::u64 getNumPoints(const std::vector<std::string>& filepaths);
		static bool loadBatch(ReadTask* task, AABB& aabb, const std::string& savePath, PointCloud::SortingMethod sortMethod);
		static LasHeader loadHeader(const std::filesystem::path& file);
		static bool loadPly(const std::string& filePath, bool useClassification = true);
		static bool loadMergeLas(const std::vector<std::string>& filepaths, const bool useClassification = true);
		static bool loadMergeLasParallel(const std::vector<std::string>& filepaths);
		static bool loadNativeCloud(const std::string& filePath);
		static bool mergeNimbusClouds(
			const std::string& rootPath, const std::string& cloudName1, const std::string& cloudName2, const std::string& mergeName, 
			const AABB& aabb, bool shouldShuffle);

	public:
		static bool saving;
		static unsigned LASClassificationSize;
		static std::unordered_map<std::string, char> LASClassification;
		static std::string LASClassificationStrings[13];
		static vec4 LASClassificationColors[13];
		static std::vector<std::string> loadedFiles;

		static float _loadProgress, _saveProgress;

		/**
		 * \brief Carga la nube de puntos especificada
		 * \param filepath ruta a la nube de puntos. Debe de contener la extension
		 * \param useClassification Si es true y la nube tiene datos de clasificacion, los cargara. En caso de ser false, no se cargar�n.
		 * \return bool dependiendo de si se ha cargado correctamente o no.
		 */
		static bool loadPointCloud(const std::string& filepath, bool useClassification = true);

		/**
		 * \brief Carga multiples nubes como si fuera una sola
		 * \param filepaths rutas a la nubes de puntos. Debe de contener la extension
		 * \param useClassification Si es true y la nube tiene datos de clasificacion, los cargara. En caso de ser false, no se cargar�n.
		 * \return bool dependiendo de si se ha cargado correctamente o no.
		*/
		static bool loadMergePointClouds(const std::vector<std::string>& filepaths, const bool useClassification = true);

		/**
		 * \brief Carga todas las nubes de puntos en formato PLY que se encuentren en la carpeta especificada
		 * \param folder carpeta en la que buscar
		 */
		static void loadAllCloudsUnderFolder(const std::string& folder);

		/**
		 * Method for saving the generated point cloud
		 *
		 * @param filename filename of the generated point cloud
		 * @param cloud cloud to save. All the subclouds of this cloud will be also saved.
		 */
		static bool savePointCloud(const std::string& filename, PointCloud& cloud);
	};
}