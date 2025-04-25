#include "CorePch.h"
#include "FileManager.h"

#define MSH_PLY_IMPLEMENTATION
#include "mshply/msh_ply.h"
#include "Graphics/Renderer.h"
#include "UI/GuiUtilities.h"
#include "laszip/laszip_api.h"
#include <future>
#include "unsuck.hpp"

bool Nimbus::FileManager::saving = false;
unsigned Nimbus::FileManager::LASClassificationSize = 13;

std::unordered_map<std::string, char> Nimbus::FileManager::LASClassification{
	{"Never Classified", 0},
	{"Unclassified", 1},
	{"Ground", 2},
	{"Low vegetation", 3},
	{"Medium vegetation", 4},
	{"High vegetation", 5},
	{"Building", 6},
	{"Low point noise", 7},
	{"Model Key-Point", 8},
	{"Water", 9},
	{"Rail", 10},
	{"Road surface", 11},
	{"Overlaped points", 12}
};

std::string Nimbus::FileManager::LASClassificationStrings[] = {
	"Never Classified",
	"Unclassified",
	"Ground",
	"Low vegetation",
	"Medium vegetation",
	"High vegetation",
	"Building",
	"Low point noise",
	"Model Key-Point",
	"Water",
	"Rail",
	"Road surface",
	"Overlaped points"
};

//std::string Nimbus::FileManager::LASClassificationStrings[] = {
//	"Asfalt",
//	"Building",
//	"Roof",
//	"Lighting",
//	"Leafs",
//	"Bark",
//	"Bollard",
//	"Bins",
//	"Phonebooth",
//	"Mailbox",
//	"Rail",
//	"Road surface",
//	"Overlaped points"
//};

vec4 Nimbus::FileManager::LASClassificationColors[] = {
	{0.52f, 0.52f, 0.52f, 1.0f},
	{0.7f, 0.7f, 0.7f, 1.0f},
	{0.51f, 0.25f, 0.0f, 1.0f},
	{0.0f, 0.45f, 0.0f, 1.0f},
	{0.0f, 0.6f, 0.0f, 1.0f},
	{0.0f, 0.75f, 0.0f, 1.0f},
	{0.0f, 0.51f, 0.76f, 1.0f},
	{0.85f, 0.0f, 0.0f, 1.0f},
	{0.85f, 0.75f, 0.0f, 1.0f},
	{0.0f, 0.0f, 0.85f, 1.0f},
	{1.0f, 0.5f, 0.0f, 1.0f},
	{1.0f, 0.5f, 0.0f, 1.0f},
	{1.0f, 0.0f, 1.0f, 1.0f}
};

float Nimbus::FileManager::_loadProgress = 0.0f;
float Nimbus::FileManager::_saveProgress = 0.0f;

glm::u64 Nimbus::FileManager::getNumPoints(const std::vector<std::string>& filepaths)
{
	glm::u64 numPoints = 0;

	for (const std::string& filepath : filepaths) {
		const std::filesystem::path path = filepath;
		LasHeader* header = new LasHeader(loadHeader(path.string()));

		if (header->_numPoints)
			numPoints += header->_numPoints;
	}

	return numPoints;
}

bool Nimbus::FileManager::loadBatch(ReadTask* task, AABB& aabb, const std::string& savePath, PointCloud::SortingMethod sortMethod)
{
	laszip_POINTER reader = nullptr;
	laszip_point* pointPointer = nullptr;

	laszip_BOOL isCompressed;
	laszip_BOOL requestReader = true;

	laszip_create(&reader);
	laszip_request_compatibility_mode(reader, requestReader);

	if (!laszip_open_reader(reader, task->_header->_filePath.c_str(), &isCompressed))
	{
		PointCloud* pointCloud = new PointCloud(task->_name);
		pointCloud->_position.resize(task->_length);
		pointCloud->_rgbColor.resize(task->_length);

		laszip_get_point_pointer(reader, &pointPointer);
		laszip_seek_point(reader, task->_offset);

		for (int i = 0; i < task->_length; i++) {
			dvec3 point;
			laszip_read_point(reader);
			laszip_get_coordinates(reader, &point[0]);

			pointCloud->_position[i] = point;
			pointCloud->_rgbColor[i] = VAO::Point::getFloatRGBColor({
				static_cast<float>(pointPointer->rgb[0]) / 65536,
				static_cast<float>(pointPointer->rgb[1]) / 65536,
				static_cast<float>(pointPointer->rgb[2]) / 65536
				});

			aabb.update(point);
		}

		laszip_close_reader(reader);

		pointCloud->_savePath = savePath;
		pointCloud->_savePath += "\\" + task->_name;
		pointCloud->spatialOrdering(sortMethod, true, false);

		delete pointCloud;

		return true;
	}

	return false;
}

Nimbus::FileManager::LasHeader Nimbus::FileManager::loadHeader(const std::filesystem::path& file)
{
	auto headerBuffer = readBinaryFile(file.string(), 0, 375);

	LasHeader header;
	header._versionMajor = headerBuffer->get<uint8_t>(24);
	header._versionMinor = headerBuffer->get<uint8_t>(25);
	header._headerSize = headerBuffer->get<uint16_t>(94);
	header._offsetToPointData = headerBuffer->get<uint32_t>(96);
	header._format = headerBuffer->get<uint8_t>(104);
	header._bytesPerPoint = headerBuffer->get<uint16_t>(105);
	header._name = file.stem().string();
	header._filePath = file.string();

	if (header._versionMajor == 1 && header._versionMinor <= 3) {
		header._numPoints = headerBuffer->get<uint32_t>(107);
	}
	else {
		header._numPoints = headerBuffer->get<uint64_t>(247);
	}

	header._scale[0] = headerBuffer->get<double>(131);
	header._scale[1] = headerBuffer->get<double>(139);
	header._scale[2] = headerBuffer->get<double>(147);
	header._offset[0] = headerBuffer->get<double>(155);
	header._offset[1] = headerBuffer->get<double>(163);
	header._offset[2] = headerBuffer->get<double>(171);
	header._min[0] = headerBuffer->get<double>(187);
	header._min[1] = headerBuffer->get<double>(203);
	header._min[2] = headerBuffer->get<double>(219);
	header._max[0] = headerBuffer->get<double>(179);
	header._max[1] = headerBuffer->get<double>(195);
	header._max[2] = headerBuffer->get<double>(211);

	return header;
}

/**
 * \brief Carga un fichero PLY
 * \param filePath ruta del fichero
 * \param useClassification si es true y el PLY tiene datos de clasificacion, se cargaran. En caso de ser false, no se cargaran.
 * \return bool dependiendo de si se ha cargado correctamente o no.
 */
bool Nimbus::FileManager::loadPly(const std::string& filePath, bool useClassification) {
	//TODO: handle UTM offset
	ImGui::InsertNotification(ImGuiToast{ ImGuiToastType::Info, _loadProgress, LocaleStrings::getInstance()->getString(POPUP_PROGRESS_OPENINGCLOUD).c_str() });
	const auto start = std::chrono::high_resolution_clock::now();

	_loadProgress = 0.0f;
	const std::filesystem::path path = filePath;
	std::string name = path.filename().string();

	bool haveColors = false;
	bool haveNormals = false;
	bool haveClassification = false;
	bool haveIntensity = false;
	bool haveScanAngle = false;

	//Nombres tipicos que toman dichos datos en un PLY. Puede ser que no se adapte a cualquier PLY.
	const char* posNames[] = { "x", "y", "z" };
	const char* colorNames[] = { "red", "green", "blue" };
	const char* normalNames[] = { "nx", "ny", "nz" };
	const char* classificationNames[] = { "scalar_Classification" };
	const char* intensityNames[] = { "scalar_Intensity" };
	const char* scanAngleNames[] = { "scalar_ScanAngleRank" };

	char vertexName[10] = "vertex";

	//Creamos los descriptores necesarios para msh_ply
	msh_ply_desc_t posDesc = {
		.element_name = vertexName,
		.property_names = posNames,
		.num_properties = 3,
		.data_type = MSH_PLY_FLOAT
	};

	msh_ply_desc_t colorDesc = {
		.element_name = vertexName,
		.property_names = colorNames,
		.num_properties = 3
	};

	msh_ply_desc_t normalDesc = {
		.element_name = vertexName,
		.property_names = normalNames,
		.num_properties = 3,
		.data_type = MSH_PLY_FLOAT
	};

	msh_ply_desc_t classificationDesc = {
		.element_name = vertexName,
		.property_names = classificationNames,
		.num_properties = 1
	};

	msh_ply_desc_t intensityDesc = {
		.element_name = vertexName,
		.property_names = intensityNames,
		.num_properties = 1,
		.data_type = MSH_PLY_FLOAT,
	};

	msh_ply_desc_t scanAngleDesc = {
		.element_name = vertexName,
		.property_names = scanAngleNames,
		.num_properties = 1,
		.data_type = MSH_PLY_FLOAT,
	};

	//Inicializamos las variables donde se almacenaran los datos y el numero de datos
	uint32_t numPos = 0, numColors = 0, numIntensity = 0, numNormals = 0, numClass = 0, numScanAngle = 0;
	float* pos = nullptr, * normals = nullptr, * intensity = nullptr, * scanAngle = nullptr;
	void* colors = nullptr, * classifications = nullptr;
	//Almacenamos tambien el formato de los colores y clasificacion. Dependiendo de si son uint o float, se tratara de una manera u otra.
	msh_ply_type_id_t colorsFormat, classificationFormat;

	//Abrimos el fichero.
	msh_ply_t* pf = msh_ply_open(path.string().c_str(), "rb");
	if (pf) {
		spdlog::info("{}: File opened", __FUNCTION__);
		//Se parsea la cabecera, obteniendo informacion de lo que hay en el fichero.
		int err = msh_ply_parse_header(pf);
		if (err != 0) {
			_loadProgress = 1.0f;
			spdlog::error("{}: error parsing header {}", __FUNCTION__, msh_ply_error_msg(err));
			return false;
		}
		spdlog::info("{}: header parsed", __FUNCTION__);

		//Consultamos en las propiedades si el PLY tiene informacion de color, normales...
		haveColors = msh_ply_has_properties(pf, &colorDesc);
		haveNormals = msh_ply_has_properties(pf, &normalDesc);
		haveClassification = msh_ply_has_properties(pf, &classificationDesc) && useClassification;
		haveIntensity = msh_ply_has_properties(pf, &intensityDesc);
		haveScanAngle = msh_ply_has_properties(pf, &scanAngleDesc);

		//Dependiendo de si tenemos informacion de dicho dato, se a�adir�n el puntero de datos al descriptor
		if (haveColors || haveClassification) {
			auto element = msh_ply_find_element(pf, vertexName);
			if (element != nullptr) {
				if (haveClassification) {
					classificationDesc.data = &classifications;
					classificationDesc.data_count = &numClass;
					classificationFormat = msh_ply_find_property(element, classificationNames[0])->type;
					classificationDesc.data_type = classificationFormat;
					msh_ply_add_descriptor(pf, &classificationDesc);
				}
				if (haveColors) {
					colorDesc.data = &colors;
					colorDesc.data_count = &numColors;
					colorsFormat = msh_ply_find_property(element, colorNames[0])->type;
					colorDesc.data_type = colorsFormat;
					msh_ply_add_descriptor(pf, &colorDesc);
				}
				if (haveNormals) {
					normalDesc.data = &normals;
					normalDesc.data_count = &numNormals;
					msh_ply_add_descriptor(pf, &normalDesc);
				}

				if (haveIntensity) {
					intensityDesc.data = &intensity;
					intensityDesc.data_count = &numIntensity;
					msh_ply_add_descriptor(pf, &intensityDesc);
				}

				if (haveScanAngle) {
					scanAngleDesc.data = &scanAngle;
					scanAngleDesc.data_count = &numScanAngle;
					msh_ply_add_descriptor(pf, &scanAngleDesc);
				}
			}
		}


		//El descriptor de posicion siempre se asigna(si no tenemos posicion, estamos jodidos, pero es lo que hay)
		posDesc.data = &pos;
		posDesc.data_count = &numPos;
		msh_ply_add_descriptor(pf, &posDesc);

		//Finalmente, leemos el fichero.
		err = msh_ply_read(pf);
		if (err != 0) {
			_loadProgress = 1.0f;
			spdlog::error("{}: error reading file {}", __FUNCTION__, msh_ply_error_msg(err));
			return false;
		}

		spdlog::info("{}: File readed", __FUNCTION__);

		//Movemos toda la informacion a las eedds necesarias para la nube
		std::vector<VAO::Point> _points[13];
		vec3 pointPos;
		std::vector<vec3> normalsVector[13];
		std::vector<float> intensityVector[13];
		std::vector<float> scanAngleVector[13];
		unsigned color = VAO::Point::get8BitRGBColor(vec3(0, 0, 0));
		unsigned classification = 2;
		float minIntensity = FLT_MAX, maxIntensity = -FLT_MAX;
		AABB _aabb[13];
		std::unordered_map<unsigned, unsigned> currentClassificationsMap;
		for (int index = 0; index < numPos; ++index) {
			_loadProgress = 0.1f + (float)index / numPos * 0.8f;

			const unsigned baseIndex = index * 3;
			if (haveClassification) {
				if (classificationFormat == MSH_PLY_FLOAT) {
					classification = static_cast<float*>(classifications)[index];
					currentClassificationsMap.insert({ classification, currentClassificationsMap.size() });
				} else {
					classification = static_cast<uint8_t*>(classifications)[index];
				}
				if (classification > 12) {
					classification = currentClassificationsMap.find(classification)->second;
				}
			}

			if (haveColors) {
				if (colorsFormat == MSH_PLY_FLOAT) {
					auto colorsFloat = static_cast<float*>(colors);
					color = VAO::Point::get8BitRGBColor(vec3(colorsFloat[baseIndex] * 255, colorsFloat[baseIndex + 1] * 255, colorsFloat[baseIndex + 2] * 255));
				} else {
					auto colorsUint = static_cast<uint8_t*>(colors);
					color = VAO::Point::get8BitRGBColor(vec3(colorsUint[baseIndex], colorsUint[baseIndex + 1],
															 colorsUint[baseIndex + 2]));
				}
			}
			if (haveNormals && baseIndex + 3 < numNormals) {
				normalsVector[classification].push_back(normalize(vec3(normals[baseIndex], normals[baseIndex + 1], normals[baseIndex + 2])));
			}
			if (haveIntensity && index < numIntensity) {
				minIntensity = std::min(minIntensity, intensity[index]);
				maxIntensity = std::max(maxIntensity, intensity[index]);
				intensityVector[classification].push_back(intensity[index]);
			}
			if (haveScanAngle && index < numScanAngle) {
				scanAngleVector[classification].push_back(scanAngle[index]);
			}
			pointPos = { pos[baseIndex], pos[baseIndex + 1], pos[baseIndex + 2] };
			_points[classification].push_back(VAO::Point{ pointPos, color });

			_aabb[classification].update(pointPos);
		}

		//Borramos las eedds usadas para msh_ply. En este punto, toda la informacion estan en la nube. Borrandolo ahora conseguimos bajar el uso de ram antes de hacer la optimizacion espacial. Se hace con free porque msh_ply es en C y usa malloc.
		msh_ply_close(pf);
		free(pos);
		if (haveNormals)
			free(normals);
		if (haveIntensity)
			free(intensity);
		if (haveClassification)
			free(classifications);
		if (haveColors)
			free(colors);
		if (haveScanAngle)
			free(scanAngle);

		//Normalizamos la intensidad
		if (haveIntensity) {
			float div = maxIntensity + minIntensity;
			for (auto& vector : intensityVector) {
				for (auto& val : vector) {
					val = (val + minIntensity) / div;
				}
			}
		}

		//Cargamos la nube en el renderer.
		_loadProgress = 0.9f;
		name.erase(name.length() - 4, name.length());
		if (haveClassification) {
			const auto cloud(new PointCloud(name));
			for (size_t i = 0; i < 13; i++) {
				if (_points[i].size() > 100) {
					auto subcloud = std::make_unique<PointCloud>(LASClassificationStrings[i], _points[i], _aabb[i]);

					if (haveNormals) {
						subcloud->setNormals(normalsVector[i]);
					}

					cloud->addSubcloud(std::move(subcloud));
					cloud->_savePath = path.string();
					cloud->_savePath = cloud->_savePath.substr(0, cloud->_savePath.find_last_of('.'));

					auto tempSubCloud = cloud->getSubcloud(LASClassificationStrings[i]);
					tempSubCloud->spatialOrdering(PointCloud::SortingMethod::Hilbert);
				}
			}
			Renderer::getInstance()->addModel(cloud);
		} else {
			const auto cloud = new PointCloud(name, _points[2], _aabb[2]);
			if (haveNormals) {
				cloud->setNormals(normalsVector[2]);
			}

			cloud->_savePath = path.string();
			cloud->_savePath = cloud->_savePath.substr(0, cloud->_savePath.find_last_of('.'));
			cloud->spatialOrdering(PointCloud::SortingMethod::Hilbert);
			Renderer::getInstance()->addModel(cloud);
		}
		const auto end = std::chrono::high_resolution_clock::now();

		_loadProgress = 1.0f;
		const auto int_s = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		spdlog::info("{}: File loaded in {} ms", __FUNCTION__, int_s.count());
		return true;
	}
	_loadProgress = 1.0f;
	spdlog::error("{}: Failed to open file {}", __FUNCTION__, path.string());
	return false;
}

/**
 * \brief Carga un fichero LAS/LAZ
 * \param filePaths rutas del fichero/s
 * \param useClassification si es true y el LAS tiene datos de clasificacion, se cargaran. En caso de ser false, no se cargaran.
 * \return bool dependiendo de si se ha cargado correctamente o no.
 */
bool Nimbus::FileManager::loadMergeLas(const std::vector<std::string>& filepaths, const bool useClassification) {
	_loadProgress = 0.f;

	ImGui::InsertNotification(ImGuiToast{ ImGuiToastType::Info, _loadProgress, LocaleStrings::getInstance()->getString(POPUP_PROGRESS_OPENINGCLOUD).c_str() });

	if (useClassification)
		spdlog::warn("LAS/LAZ load doesn't support classification right now");

	const auto start = std::chrono::high_resolution_clock::now();
	laszip_POINTER reader = nullptr;
	u16 numFiles = filepaths.size();

	std::queue<std::string> cloudNames;
	AABB fullCloudAABB;
	std::string rootPath;
	std::string name;
	u64 totalNumPoints = 0;

	for (u16 currentFile = 0; currentFile < numFiles; ++currentFile) {
		const std::filesystem::path path = filepaths[currentFile];
		rootPath = path.parent_path().string();
		name = path.stem().string();

		laszip_create(&reader);
		laszip_BOOL* isCompressed = new laszip_BOOL(1);
		if (!laszip_open_reader(reader, path.string().c_str(), isCompressed)) {
			laszip_header* header;
			laszip_get_header_pointer(reader, &header);

			laszip_point* pointR;
			laszip_get_point_pointer(reader, &pointR);

			u64 pointsNumber = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);
			totalNumPoints += pointsNumber;

			auto scaleFactor = vec3(header->x_scale_factor, header->y_scale_factor, header->z_scale_factor);
			auto offset = vec3(header->x_offset, header->y_offset, header->z_offset);

			u64 remainingPoints = pointsNumber;
			f64 pointsRead = 0;
			u16 pointId = 0;
			while (remainingPoints > 0) {
				std::string chunkName = name + "_" + std::to_string(pointId++);
				cloudNames.push(chunkName);
				const auto cloud = new PointCloud(chunkName);
				std::vector<vec3>& points = cloud->_position;
				std::vector<u32>& colors = cloud->_rgbColor;
				u64 pointsToRead = std::min(pointsPerBatch, remainingPoints);
				remainingPoints -= pointsToRead;
				for (u64 i = 0; i < pointsToRead; ++i) {
					laszip_read_point(reader);
					auto point = vec3(
						pointR->X * scaleFactor.x,
						pointR->Y * scaleFactor.y,
						pointR->Z * scaleFactor.z);
					fullCloudAABB.update(point);
					points.push_back(point);
					//El color en LAS/LAZ viene codificado en uint16. Para pasarlo a float lo dividimos entre 2^16
					uint32_t codedColor = VAO::Point::getFloatRGBColor({ (float)pointR->rgb[0] / 65536, (float)pointR->rgb[1] / 65536, (float)pointR->rgb[2] / 65536 });
					colors.push_back(codedColor);
					_loadProgress = (float)currentFile / numFiles + (pointsRead++ / (float)pointsNumber) / numFiles;
				}

				cloud->setPosition({ offset.x, offset.y, offset.z });
				cloud->_name = chunkName;
				cloud->_savePath = rootPath;
				cloud->_savePath += "\\" + chunkName;
				cloud->saveBinaryFile(cloud->_savePath);
				delete cloud;
			}
			laszip_close_reader(reader);
		}
	}
	_loadProgress = 1.0f;

	static float sortingProgress = 0;
	ImGui::InsertNotification(ImGuiToast{ ImGuiToastType::Info, sortingProgress, LocaleStrings::getInstance()->getString(POPUP_PROGRESS_SORTINGCLOUD).c_str() });

	u32 numCloudNames = cloudNames.size();
	for (u32 i = 0; i < numCloudNames; ++i) {
		std::string cloudName = cloudNames.front();
		cloudNames.pop();
		cloudNames.push(cloudName);
		auto cloud = new PointCloud(cloudName);
		cloud->_aabb.update(fullCloudAABB);
		cloud->_savePath = rootPath + "\\" + cloudName;
		cloud->loadPositionsFromBinaryFile(cloud->_savePath);
		cloud->loadRGBFromBinaryFile(cloud->_savePath);
		cloud->spatialOrdering(PointCloud::SortingMethod::Hilbert, true, numCloudNames == 1);
		delete cloud;
		sortingProgress += 1.0f / cloudNames.size();
	}
	sortingProgress = 1.0f;

	static float mergingProgress = 0;
	ImGui::InsertNotification(ImGuiToast{ ImGuiToastType::Info, mergingProgress, LocaleStrings::getInstance()->getString(POPUP_PROGRESS_MERGINGCLOUD).c_str() });

	std::vector<std::future<std::string>> asyncCalls;
	u32 mergeId = 0;
	u32 totalMerges = cloudNames.size() - 1;
	u32 mergesCompleted = 0;
	do {
		for (auto& future : asyncCalls) {
			auto result = future.get();
			if (!result.empty()) {
				cloudNames.push(result);
				mergesCompleted++;
				mergingProgress = (float)mergesCompleted / totalMerges;
			}
		}
		asyncCalls.clear();
		while (cloudNames.size() > 1 && asyncCalls.size() < 20) {
			auto cloudName1 = cloudNames.front();
			cloudNames.pop();
			auto cloudName2 = cloudNames.front();
			cloudNames.pop();
			std::string mergeName = "Merge_" + std::to_string(mergeId++);
			asyncCalls.push_back(std::async(std::launch::async,
											[=]() {
				if (mergeNimbusClouds(rootPath, cloudName1, cloudName2, mergeName, fullCloudAABB, mergeId == totalMerges))
					return mergeName;
				return std::string();
			}));
		}
	} while (!asyncCalls.empty());
	mergingProgress = 1.0f;

	auto cloudName = cloudNames.front();
	auto cloud = new PointCloud(cloudName);
	std::filesystem::rename(rootPath + "\\" + cloudName + ".NimbusCloud", rootPath + "\\" + name + ".NimbusCloud");
	std::filesystem::rename(rootPath + "\\" + cloudName + ".NimbusCloudPos", rootPath + "\\" + name + ".NimbusCloudPos");
	std::filesystem::rename(rootPath + "\\" + cloudName + ".NimbusCloudRGB", rootPath + "\\" + name + ".NimbusCloudRGB");
	cloud->loadBinaryFile(rootPath + "\\" + name);
	cloud->_name = name;
	cloud->_savePath = rootPath + "\\" + name;
	//cloud->computeMetrics("Hilbert");
	cloud->saveMetadata(rootPath + "\\" + name);
	cloud->needUpdate();
	Renderer::getInstance()->addModel(cloud);
	const auto end = std::chrono::high_resolution_clock::now();

	const auto int_s = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	spdlog::info("{}: Files loaded in {} ms", __FUNCTION__, int_s.count());
	return true;
}

bool Nimbus::FileManager::loadMergeLasParallel(const std::vector<std::string>& filepaths)
{
	_loadProgress = 0.f;

	ImGui::InsertNotification(ImGuiToast{ ImGuiToastType::Info, _loadProgress, LocaleStrings::getInstance()->getString(POPUP_PROGRESS_OPENINGCLOUD).c_str() });

	const auto start = std::chrono::high_resolution_clock::now();

	std::vector<std::string> cloudNames;
	std::vector<std::string> mergeCloudNames;
	std::vector<LasHeader*> cloudHeaders;
	std::vector<ReadTask*> readTasks;
	AABB fullCloudAABB;
	std::string name;
	u32 maxNumThreads = std::thread::hardware_concurrency() - 1;
	glm::u64 optimalPointsPerBatch = getNumPoints(filepaths) / maxNumThreads * 2;

	for (const std::string& filepath : filepaths) {
		const std::filesystem::path path = filepath;
		LasHeader* header = new LasHeader(loadHeader(path.string()));

		if (header->_numPoints)
		{
			cloudHeaders.push_back(header);
			name = header->_name;

			// Include tasks according to the number of points
			u64 remainingPoints = header->_numPoints;
			i64 offset = 0;
			u16 pointId = 0;

			while (remainingPoints > 0) {
				i64 pointsToRead = static_cast<i64>(std::min(optimalPointsPerBatch, remainingPoints));
				remainingPoints -= pointsToRead;
				std::string chunkName = header->_name + "_" + std::to_string(pointId++);

				readTasks.push_back(new ReadTask{
					._header = cloudHeaders.back(), ._name = chunkName, ._rootPath = path.parent_path().string(),
					._offset = offset, ._length = pointsToRead
					});

				offset += pointsToRead;
			}
		}
	}

	//
	std::string rootPath = readTasks.front()->_rootPath;

	// Thread and point cloud variables
	std::vector<std::future<std::tuple<std::string, AABB, ReadTask*>>> readCalls;
	u32 mergeId = 0;
	size_t totalReadTasks = readTasks.size() / 2;
	u32 readsCompleted = 0;

	do
	{
		for (auto& future : readCalls) {
			auto result = future.get();
			auto chunkName = std::get<std::string>(result);

			readsCompleted++;

			if (!chunkName.empty()) {
				AABB aabb = std::get<AABB>(result);
				delete std::get<ReadTask*>(result);

				fullCloudAABB.update(aabb);

				mergeCloudNames.push_back(chunkName);
				_loadProgress = static_cast<float>(readsCompleted) / static_cast<float>(totalReadTasks);
			}

		}

		readCalls.clear();

		while (!readTasks.empty() && readCalls.size() < maxNumThreads) {
			ReadTask* readTask = readTasks.back();
			readTasks.erase(readTasks.begin() + readTasks.size() - 1);
			std::string mergeName = "Merge_" + std::to_string(mergeId++);

			readCalls.push_back(std::async(std::launch::async,
				[=] {
					AABB aabb;
					if (loadBatch(readTask, aabb, rootPath, PointCloud::SortingMethod::Hilbert))
						return std::make_tuple(readTask->_name, aabb, readTask);
					return std::make_tuple(std::string(""), aabb, readTask);
				}));
		}
	}
	while (!readCalls.empty());

	_loadProgress = 1.0f;

	//static float sortingProgress = 0;
	//ImGui::InsertNotification(ImGuiToast{ ImGuiToastType::Info, sortingProgress, LocaleStrings::getInstance()->getString(POPUP_PROGRESS_SORTINGCLOUD).c_str() });

	//size_t numCloudNames = cloudNames.size();

	//#pragma omp parallel for
	//for (int i = 0; i < static_cast<int>(numCloudNames); ++i) {
	//	const std::string& cloudName = cloudNames[i];
	//	auto cloud = new PointCloud(cloudName);
	//	cloud->_aabb.update(fullCloudAABB);
	//	cloud->_savePath = rootPath;
	//	cloud->_savePath += "\\" + cloudName;
	//	cloud->loadPositionsFromBinaryFile(cloud->_savePath);
	//	cloud->loadRGBFromBinaryFile(cloud->_savePath);
	//	cloud->spatialOrdering(PointCloud::SortingMethod::Hilbert, true, numCloudNames == 1);
	//	delete cloud;

	//	#pragma omp critical
	//	mergeCloudNames.emplace(cloudName);

	//	sortingProgress += 1.0f / static_cast<float>(cloudNames.size());
	//}

	//sortingProgress = 1.0f;

	static float mergingProgress = 0;
	ImGui::InsertNotification(ImGuiToast{ ImGuiToastType::Info, mergingProgress, LocaleStrings::getInstance()->getString(POPUP_PROGRESS_MERGINGCLOUD).c_str() });

	std::vector<std::future<std::string>> mergeCalls;
	size_t totalMerges = mergeCloudNames.size() - 1;

	mergeId = 0;
	do {
		size_t numClouds = mergeCloudNames.size();
		if (numClouds % 2 != 0)
			--numClouds;

		#pragma omp parallel for
		for (int idx = 0; idx < numClouds; idx += 2)
		{
			std::string cloudName1 = mergeCloudNames[idx];
			std::string cloudName2 = mergeCloudNames[idx + 1];

			std::string mergeName = "Merge_" + std::to_string(idx / 2 + mergeId);
			mergeNimbusClouds(rootPath, cloudName1, cloudName2, mergeName, fullCloudAABB, numClouds == 2);

			#pragma omp critical
			mergeCloudNames.push_back(mergeName);
		}

		mergeId += numClouds / 2;
		mergingProgress = static_cast<float>(mergeId) / static_cast<float>(totalMerges);
		mergeCloudNames.erase(mergeCloudNames.begin(), mergeCloudNames.begin() + numClouds);
	} while (mergeCloudNames.size() > 1);
	//do {
	//	for (auto& future : mergeCalls) {
	//		auto result = future.get();
	//		if (!result.empty()) {
	//			mergeCloudNames.push(result);
	//			mergesCompleted++;
	//			mergingProgress = static_cast<float>(mergesCompleted) / static_cast<float>(totalMerges);
	//		}
	//	}

	//	mergeCalls.clear();
	//	while (mergeCloudNames.size() > 1 && mergeCalls.size() < maxNumThreads) {
	//		std::string cloudName1 = mergeCloudNames.front();
	//		mergeCloudNames.pop();
	//		std::string cloudName2 = mergeCloudNames.front();
	//		mergeCloudNames.pop();

	//		std::string mergeName = "Merge_" + std::to_string(mergeId++);
	//		mergeCalls.push_back(std::async(std::launch::async,
	//			[=]() {
	//				if (mergeNimbusClouds(cloudName1, cloudName2, mergeName, fullCloudAABB, rootPath, mergeId == totalMerges))
	//					return mergeName;
	//				return std::string();
	//			}));
	//	}
	//} while (!mergeCalls.empty());
	//mergingProgress = 1.0f;

	auto cloudName = mergeCloudNames.front();
	auto cloud = new PointCloud(cloudName);
	std::filesystem::rename(rootPath + "\\" + cloudName + ".NimbusCloud", rootPath + "\\" + name + ".NimbusCloud");
	std::filesystem::rename(rootPath + "\\" + cloudName + ".NimbusCloudPos", rootPath + "\\" + name + ".NimbusCloudPos");
	std::filesystem::rename(rootPath + "\\" + cloudName + ".NimbusCloudRGB", rootPath + "\\" + name + ".NimbusCloudRGB");
	cloud->loadBinaryFile(rootPath + "\\" + name);
	cloud->_name = name;
	cloud->_savePath = rootPath + "\\" + name;
	cloud->saveMetadata(rootPath + "\\" + name);
	cloud->needUpdate();
	Renderer::getInstance()->addModel(cloud);
	const auto end = std::chrono::high_resolution_clock::now();

	const auto int_s = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	spdlog::info("{}: Files loaded in {} ms", __FUNCTION__, int_s.count());

	for (LasHeader* header : cloudHeaders)
		delete header;

	return true;
}

bool Nimbus::FileManager::loadNativeCloud(const std::string& filePath) {
	const auto path = std::filesystem::path(filePath);
	const auto cloud = new PointCloud(path.filename().string(), {});
	cloud->_savePath = path.string();
	cloud->_savePath = cloud->_savePath.substr(0, cloud->_savePath.find_last_of('.'));
	cloud->loadBinaryFile(cloud->_savePath);
	Renderer::getInstance()->addModel(cloud);
	return true;
}

template<typename T>
void reorder(std::vector<u32>& order, std::vector<T>& v) {
	std::vector<T> temp(v.size());
	for (size_t i = 0; i < v.size(); ++i) {
		temp[i] = v[order[i]];
	}
	v = std::move(temp);
}

bool Nimbus::FileManager::mergeNimbusClouds(
	const std::string& rootPath,
	const std::string& cloudName1,
	const std::string& cloudName2,
	const std::string& mergeName,
	const AABB& aabb,
	bool shouldShuffle) {
	// Constants for batch processing
	const size_t BATCH_SIZE = 65536; // Adjust based on your system's cache size
	const size_t STREAM_BUFFER_SIZE = 1 << 20; // 1MB

	// Initialize clouds
	PointCloud cloud1(cloudName1);
	PointCloud cloud2(cloudName2);
	std::string cloudPath1 = rootPath + "\\" + cloudName1;
	std::string cloudPath2 = rootPath + "\\" + cloudName2;

	// Load metadata
	cloud1.loadBinaryFile(cloudPath1);
	cloud1.closeReadStreams();
	cloud2.loadBinaryFile(cloudPath2);
	cloud2.closeReadStreams();

	// Prepare merged cloud
	AABB mergedAABB = aabb;
	//AABB mergedAABB = cloud1.getAABB();
	//mergedAABB.update(cloud2.getAABB());
	PointCloud mergedCloud(mergeName, mergedAABB);
	mergedCloud._numPoints = cloud1._numPoints + cloud2._numPoints;
	mergedCloud._meshletSize = Nimbus::PointCloud::computeMeshletSize(mergedCloud._numPoints);
	auto meshletSize = mergedCloud._meshletSize;

	// Buffers must persist as long as streams exist
	thread_local std::vector<char> inputBuffer1(STREAM_BUFFER_SIZE);
	thread_local std::vector<char> inputBuffer2(STREAM_BUFFER_SIZE);
	thread_local std::vector<char> rgbBuffer1(STREAM_BUFFER_SIZE);
	thread_local std::vector<char> rgbBuffer2(STREAM_BUFFER_SIZE);
	thread_local std::vector<char> outputPosBuffer(STREAM_BUFFER_SIZE);
	thread_local std::vector<char> outputRgbBuffer(STREAM_BUFFER_SIZE);

	// Input and output streams with buffers
	std::ifstream cloud1Pos(cloudPath1 + ".NimbusCloudPos", std::ios::binary);
	std::ifstream cloud2Pos(cloudPath2 + ".NimbusCloudPos", std::ios::binary);
	std::ifstream cloud1Rgb(cloudPath1 + ".NimbusCloudRGB", std::ios::binary);
	std::ifstream cloud2Rgb(cloudPath2 + ".NimbusCloudRGB", std::ios::binary);

	std::ofstream mergedCloudPos(rootPath + "\\" + mergedCloud._name + ".NimbusCloudPos", std::ios::binary);
	std::ofstream mergedCloudRgb(rootPath + "\\" + mergedCloud._name + ".NimbusCloudRGB", std::ios::binary);

	// Associate buffers with streams
	cloud1Pos.rdbuf()->pubsetbuf(inputBuffer1.data(), STREAM_BUFFER_SIZE);
	cloud2Pos.rdbuf()->pubsetbuf(inputBuffer2.data(), STREAM_BUFFER_SIZE);
	cloud1Rgb.rdbuf()->pubsetbuf(rgbBuffer1.data(), STREAM_BUFFER_SIZE);
	cloud2Rgb.rdbuf()->pubsetbuf(rgbBuffer2.data(), STREAM_BUFFER_SIZE);
	mergedCloudPos.rdbuf()->pubsetbuf(outputPosBuffer.data(), STREAM_BUFFER_SIZE);
	mergedCloudRgb.rdbuf()->pubsetbuf(outputRgbBuffer.data(), STREAM_BUFFER_SIZE);

	if (!cloud1Pos.good() || !cloud2Pos.good() || !cloud1Rgb.good() || !cloud2Rgb.good() ||
		!mergedCloudPos.good() || !mergedCloudRgb.good()) {
		return false;
	}

	//
	std::vector<vec3> batch1Pos, batch2Pos;
	std::vector<u32> batch1Rgb, batch2Rgb;
	std::vector<u32> batch1Hilbert, batch2Hilbert;

	batch1Pos.reserve(BATCH_SIZE);
	batch2Pos.reserve(BATCH_SIZE);
	batch1Rgb.reserve(BATCH_SIZE);
	batch2Rgb.reserve(BATCH_SIZE);
	batch1Hilbert.reserve(BATCH_SIZE);
	batch2Hilbert.reserve(BATCH_SIZE);

	// Reading a batch of points
	auto readBatch = [](std::ifstream& posStream, std::ifstream& rgbStream,
		std::vector<vec3>& posBatch, std::vector<u32>& rgbBatch,
		std::vector<u32>& hilbertBatch, const AABB& aabb) {
			posBatch.clear();
			rgbBatch.clear();
			hilbertBatch.clear();

			vec3 pos;
			u32 rgb;
			for (size_t i = 0; i < BATCH_SIZE; ++i) {
				posStream.read(reinterpret_cast<char*>(&pos), sizeof(vec3));
				rgbStream.read(reinterpret_cast<char*>(&rgb), sizeof(u32));
				if (!posStream || !rgbStream) break;

				posBatch.push_back(pos);
				rgbBatch.push_back(rgb);
				hilbertBatch.push_back(PointCloud::hilbert3D(pos, aabb));
			}
			return !posBatch.empty();
		};

	// Read initial batches
	bool hasMore1 = readBatch(cloud1Pos, cloud1Rgb, batch1Pos, batch1Rgb, batch1Hilbert, mergedAABB);
	bool hasMore2 = readBatch(cloud2Pos, cloud2Rgb, batch2Pos, batch2Rgb, batch2Hilbert, mergedAABB);

	size_t idx1 = 0, idx2 = 0;
	u32 pointId = 0;
	mergedCloud._meshlets.emplace_back();
	AABB* currentMeshletAABB = &mergedCloud._meshlets.back().aabb;
	std::vector<vec3> currentMeshletPosition;
	std::vector<u32> currentMeshletColor;
	currentMeshletPosition.reserve(meshletSize);
	currentMeshletColor.reserve(meshletSize);
	std::vector<u32> shuffling(meshletSize);

	// Merge loop
	while (hasMore1 || hasMore2) {
		// Process the point
		if (hasMore1 && (!hasMore2 || batch1Hilbert[idx1] <= batch2Hilbert[idx2])) {
			currentMeshletAABB->update(batch1Pos[idx1]);
			currentMeshletPosition.push_back(batch1Pos[idx1]);
			currentMeshletColor.push_back(batch1Rgb[idx1]);

			if (++idx1 >= batch1Pos.size()) {
				hasMore1 = readBatch(cloud1Pos, cloud1Rgb, batch1Pos, batch1Rgb, batch1Hilbert, mergedAABB);
				idx1 = 0;
			}
		}
		else {
			currentMeshletAABB->update(batch2Pos[idx2]);
			currentMeshletPosition.push_back(batch2Pos[idx2]);
			currentMeshletColor.push_back(batch2Rgb[idx2]);

			if (++idx2 >= batch2Pos.size()) {
				hasMore2 = readBatch(cloud2Pos, cloud2Rgb, batch2Pos, batch2Rgb, batch2Hilbert, mergedAABB);
				idx2 = 0;
			}
		}

		// Handle meshlet completion
		if (++pointId == meshletSize) {
			pointId = 0;
			if (hasMore1 || hasMore2) {
				mergedCloud._meshlets.emplace_back();
				currentMeshletAABB = &mergedCloud._meshlets.back().aabb;
			}

			if (shouldShuffle) {
				std::iota(shuffling.begin(), shuffling.end(), 0);
				std::ranges::shuffle(shuffling, std::default_random_engine(mergedCloud._numPoints));
				reorder(shuffling, currentMeshletPosition);
				reorder(shuffling, currentMeshletColor);
			}

			mergedCloudPos.write(reinterpret_cast<char*>(currentMeshletPosition.data()), currentMeshletPosition.size() * sizeof(vec3));
			mergedCloudRgb.write(reinterpret_cast<char*>(currentMeshletColor.data()), currentMeshletColor.size() * sizeof(u32));

			currentMeshletPosition.clear();
			currentMeshletColor.clear();
		}
	}

	// Close all streams
	cloud1Pos.close();
	cloud2Pos.close();
	cloud1Rgb.close();
	cloud2Rgb.close();
	mergedCloudPos.close();
	mergedCloudRgb.close();

	// Finalize merged cloud
	mergedCloud.computeMetrics("Hilbert");
	mergedCloud.saveMetadata(rootPath + "\\" + mergedCloud._name);

	// Cleanup
	std::filesystem::remove(cloudPath1 + ".NimbusCloud");
	std::filesystem::remove(cloudPath2 + ".NimbusCloud");
	std::filesystem::remove(cloudPath1 + ".NimbusCloudPos");
	std::filesystem::remove(cloudPath2 + ".NimbusCloudPos");
	std::filesystem::remove(cloudPath1 + ".NimbusCloudRGB");
	std::filesystem::remove(cloudPath2 + ".NimbusCloudRGB");

	return true;
}

bool Nimbus::FileManager::loadPointCloud(const std::string& filepath, const bool useClassification) {
	const auto filePath = std::filesystem::path(filepath);
	if (exists(filePath)) {
		std::string extension = filePath.extension().string();
		try {
			//Pese a que el fichero no representa el tamano real de la nube, nos sirve de aproximación.
			const auto fileSize = file_size(filePath);
			if (extension == ".ply") {
				if (fileSize > INT_MAX * 6) {
					spdlog::warn("File is too big. There could be problems loading it. Try to use the LAS format instead");
				}
				return loadPly(filePath.string(), useClassification);
			}
			if (extension == ".las" || extension == ".laz") { //El metodo LAS comprueba si está comprimido o no, por eso se usa el mismo.
				std::vector<string> t;
				t.push_back(filepath);
				return loadMergeLasParallel(t);
			}if (extension == ".NimbusCloud")
				return loadNativeCloud(filePath.string());
		} catch (std::exception& e) {
			spdlog::warn("Cannot load {} file: {}", extension, e.what());
			GuiUtilities::InsertToast(FILEDIALOG_OPEN_ERRORCLOUD, ImGuiToastType::Error);
		}
	} else {
		spdlog::error("{} doesn't exist", filePath.filename().string());
	}
	return false;
}

bool Nimbus::FileManager::loadMergePointClouds(const std::vector<std::string>& filepaths, const bool useClassification) {
	for (auto& filepath : filepaths) {
		const auto filePath = std::filesystem::path(filepath);
		if (exists(filePath)) {
			std::string extension = filePath.extension().string();
			try {
				if (extension != ".las" && extension != ".laz") { //El metodo LAS comprueba si está comprimido o no, por eso se usa el mismo.
					throw std::runtime_error("All clouds must be in LAS/LAZ format");
				}
			} catch (std::exception& e) {
				spdlog::warn("Cannot load {} file: {}", extension, e.what());
				GuiUtilities::InsertToast(FILEDIALOG_OPEN_ERRORCLOUD, ImGuiToastType::Error);
				return false;
			}
		}
	}

	loadMergeLasParallel(filepaths);
	return true;
}

void Nimbus::FileManager::loadAllCloudsUnderFolder(const std::string& folder) {
	for (const auto& file : std::filesystem::directory_iterator(folder)) {
		if (!file.is_directory()) {
			auto path = file.path().string();
			loadPointCloud(path);
		}
	}
}

bool Nimbus::FileManager::savePointCloud(const std::string& filename, PointCloud& cloud) {
	const auto start = std::chrono::high_resolution_clock::now();

	const bool haveNormals = cloud.haveNormals();
	// const auto& fusionData = cloud.getFusionData();//TODO: refactor fusionData

	//Nombres tipicos que toman dichos datos en un PLY. Puede ser que no se adapte a cualquier PLY.
	char vertexName[10] = "vertex";
	const char* posNames[] = { "x", "y", "z" };
	const char* colorNames[] = { "red", "green", "blue" };
	const char* normalNames[] = { "nx", "ny", "nz" };
	const char* classificationNames[] = { "scalar_Classification" };
	std::vector<std::string> fusionDataStrings;
	std::vector<const char*> fusionDataNames;

	uint32_t totalPointNumber = cloud.getNumberOfPoints();
	uint32_t classificationNumber = totalPointNumber;
	uint32_t normalsNumber = cloud.getNormals().size();
	std::vector<uint32_t> fusionDataNumber[(u8)Attribute::NumAttributes];

	//Abrimos el fichero con msh_ply para poder anadir los descriptores.
	msh_ply_t* pf = msh_ply_open(filename.c_str(), "wb");

	if (pf) {
		//Trasladamos nuestros datos de la nube a nuevos vectores para darles el formato necesario para exportar
		//TODO: Asociar directamente las eedds de la nube, saltandose este paso. Requeriria de un refactor de la nube.
		auto points = new float[totalPointNumber * 3];
		auto colors = new float[totalPointNumber * 3];
		saving = true;

		const auto& positions = cloud.getPointsPosition();
		const auto& pointsColor = cloud.getPointsColor();

		for (int i = 0; i < totalPointNumber; ++i) {
			const int baseIndex = i * 3;
			points[baseIndex] = positions[i].x;
			points[baseIndex + 1] = positions[i].y;
			points[baseIndex + 2] = positions[i].z;
			const auto color = VAO::Point::getRGBVec4(pointsColor[i]);
			colors[baseIndex] = color.x;
			colors[baseIndex + 1] = color.y;
			colors[baseIndex + 2] = color.z;
		}

		//Creamos los descriptores necesarios para msh_ply
		msh_ply_desc_t posDesc = {
			.element_name = vertexName,
			.property_names = posNames,
			.num_properties = 3,
			.data_type = MSH_PLY_FLOAT,
			.data = &points,
			.data_count = &totalPointNumber
		};
		msh_ply_add_descriptor(pf, &posDesc);

		msh_ply_desc_t colorDesc = {
			.element_name = vertexName,
			.property_names = colorNames,
			.num_properties = 3,
			.data_type = MSH_PLY_FLOAT,
			.data = &colors,
			.data_count = &totalPointNumber
		};
		msh_ply_add_descriptor(pf, &colorDesc);

		msh_ply_desc_t normalDesc;
		auto normals = cloud.getNormals().data();
		if (haveNormals) {
			normalDesc = {
				.element_name = vertexName,
				.property_names = normalNames,
				.num_properties = 3,
				.data_type = MSH_PLY_FLOAT,
				.data = &normals,
				.data_count = &normalsNumber
			};
			msh_ply_add_descriptor(pf, &normalDesc);
		}

		//Escribimos el fichero en disco
		const auto result = msh_ply_write(pf);
		if (result != MSH_PLY_NO_ERR) {
			spdlog::error("{}:{}", __FUNCTION__, msh_ply_error_msg(result));
			return false;
		}

		//Liberamos los recursos reservados
		msh_ply_close(pf);
		delete[] points;
		delete[] colors;

		const auto end = std::chrono::high_resolution_clock::now();

		const auto int_s = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		spdlog::info("{}: File saved in {} ms", __FUNCTION__, int_s.count());
		return true;
	}

	return false;
}
