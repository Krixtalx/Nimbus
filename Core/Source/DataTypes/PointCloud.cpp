#include "CorePch.h"
#include "PointCloud.h"

#include <glm/ext/matrix_projection.hpp>

#include "Geometry/Operators2D.h"
#include "Geometry/Operators3D.h"
#include "Graphics/MaterialList.h"
#include "Graphics/Renderer.h"
#include "Graphics/Shaders/RenderingShader.h"
#include "IO/FileManager.h"
#include "Managers/InputManager.h"
#include "Managers/MemoryManager.h"
#include "nanoflann.hpp"
#include "Geometry/AABB.h"
#include "Utilities/ChronoUtilities.h"


Nimbus::PointCloud::PointCloud(const std::string& name) : Model3D(name), _meanMeshletExtent(0), _classification(0) {
	_material = MaterialList::getInstance()->isMaterialAvailable("PointCloudMaterial");
	std::ranges::fill(_availableDataBands, 0);
}

Nimbus::PointCloud::PointCloud(const std::string& name, const AABB& aabb, const u32& classification, const vec3& pos,
							const vec3& rot, const vec3& scale) : Model3D(name, aabb, pos, rot, scale),
	_meanMeshletExtent(0), _classification(classification) {
	std::ranges::fill(_availableDataBands, 0);
	_material = MaterialList::getInstance()->isMaterialAvailable("PointCloudMaterial");
}

Nimbus::PointCloud::PointCloud(const std::string& name, const std::vector<VAO::Point>& points, const AABB& aabb, const u32& classification, const vec3& pos, const vec3& rot, const vec3& scale) :
	Model3D(name, aabb, pos, rot, scale), _classification(classification) {
	_position.reserve(points.size());
	_rgbColor.reserve(points.size());
	std::ranges::fill(_availableDataBands, 0);
	for (auto& point : points) {
		_position.push_back(point._point);
		_rgbColor.push_back(point._rgb);
	}
	_material = MaterialList::getInstance()->isMaterialAvailable("PointCloudMaterial");
}

Nimbus::PointCloud::~PointCloud() = default;

/**
 * Llamada de dibujado de una nube de puntos. Se encarga de dibujar todas las subnubes y a si misma si fuera el caso.
 *
 * \param viewProjMatrix View projection Matrix
 */
void Nimbus::PointCloud::drawModel(dmat4 viewProjMatrix) {
	throw std::logic_error("The method is no longer usable.");
	if (_enabled) {
		updateCloud(); //Esta funcion se llamar� desde el loop de renderizado, por lo que es seguro que se ejecutar� en el hilo principal. Por ello, aprovechamos para actualizar los recursos OpenGL que esten pendientes de actualizar.

		viewProjMatrix = viewProjMatrix * getModelMatrix();

		RenderingShader* renderShader = _material->getAssociatedShader();
		renderShader->setUniform("meshletSize", _meshletSize);
		renderShader->setUniform("classColor", FileManager::LASClassificationColors[_classification]);
		renderShader->setUniform("mModelViewProj", viewProjMatrix);
		_material->applyMaterial();

		if (!_position.empty()) {
			_GPUResources.bindBuffer(GPUResources::SSBOSlots::PositionFrame1, GPUResources::GPUBinding::Position);
			_GPUResources.bindBuffer(GPUResources::SSBOSlots::AttributeFrame1, GPUResources::GPUBinding::Attribute);
			if (getMeshletNumber() > 0)
				_GPUResources.bindBuffer(GPUResources::SSBOSlots::CullingBuffer, GPUResources::GPUBinding::CullingBuffer);
			/*if (!_enableDebugView) {
				if (_renderMode != _renderModes[0]) {
					if (_renderMode == _renderModes[1]) {
						_GPUResources.bindBuffer(GPUResources::SSBOSlots::Thermal, GPUResources::GPUBinding::FusionData);
					} else if (_renderMode == _renderModes[2]) {
						_GPUResources.bindBuffer(GPUResources::SSBOSlots::Multispectral, GPUResources::GPUBinding::FusionData);
					} else if (_renderMode == _renderModes[3]) {
						_GPUResources.bindBuffer(GPUResources::SSBOSlots::Hyperspectral, GPUResources::GPUBinding::FusionData);
					}
					_material->setSubroutine(ShaderEnum::VertexSubroutines::VERTEX_COLOR_SOURCE, "useFusionDataAsColor");
				} else {
					_material->setSubroutine(ShaderEnum::VertexSubroutines::VERTEX_COLOR_SOURCE, "usePointColor");
				}
			}*/
			glDrawArrays(GL_POINTS, 0, _position.size());
		}

		for (const auto& subcloud : _subClouds) {
			subcloud->drawModel(viewProjMatrix);
		}
	}
}

/**
 * Morton Code based compare function
 */
 //bool compareFunction(const std::pair<u32, vec3>& a, const std::pair<u32, vec3>& b) {
 //	const uint_fast64_t aVal = libmorton::morton3D_64_encode(a.second.x * 1000, a.second.y * 1000, a.second.z * 1000);
 //	const uint_fast64_t bVal = libmorton::morton3D_64_encode(b.second.x * 1000, b.second.y * 1000, b.second.z * 1000);
 //	return aVal < bVal;
 //}

// Expands a 10-bit integer into 30 bits
// by inserting 2 zeros after each bit.
u32 expandBits(u32 x) {
	x = (x * 0x00010001u) & 0xFF0000FFu;
	x = (x * 0x00000101u) & 0x0F00F00Fu;
	x = (x * 0x00000011u) & 0xC30C30C3u;
	x = (x * 0x00000005u) & 0x49249249u;
	return x;
}

// Calculates a 30-bit Morton code for the
// given 3D point located within the unit cube [0,1].
u32 morton3D(float x, float y, float z) {
	x = x * 1023.f + 0.5f;
	y = y * 1023.f + 0.5f;
	z = z * 1023.f + 0.5f;
	const u32 xx = expandBits((u32)x);
	const u32 yy = expandBits((u32)y);
	const u32 zz = expandBits((u32)z);
	return xx * 4 + yy * 2 + zz;
}

bool compareFunction(const std::pair<u32, u32>& a, const std::pair<u32, u32>& b) {
	return a.second < b.second;
}

template <typename Derived>
struct PointCloudAdaptor {

	Derived* cloud;  //!< A const ref to the data set origin

	/// The constructor that sets the data set source
	PointCloudAdaptor(Derived* obj_) : cloud(obj_) {}

	/// CRTP helper method
	const Derived* derived() const { return cloud; }

	// Must return the number of data points
	size_t kdtree_get_point_count() const {
		return cloud->getPointsPosition().size();
	}

	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate
	// value
	float kdtree_get_pt(const size_t idx, const size_t dim) const {
		return cloud->getPointsPosition()[idx][dim];
	}

	// Optional bounding-box computation: return false to default to a standard
	// bbox computation loop.
	//   Return true if the BBOX was already computed by the class and returned
	//   in "bb" so it can be avoided to redo it again. Look at bb.size() to
	//   find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX& /*bb*/) const {
		return false;
	}

};  // end of PointCloudAdaptor

static constexpr u32 mortonToHilbertTable[] = {
	0x00000006, 0x20000005, 0x60000001, 0x40000007, 0xe0000001, 0xc000000c, 0x80000005, 0xa000000e,
	0x40000000, 0xa000000a, 0x6000000c, 0x8000000d, 0x20000004, 0xc000000e, 0x00000007, 0xe0000008,
	0x80000019, 0xe000001a, 0x6000001b, 0x00000010, 0xa0000012, 0xc0000013, 0x4000001c, 0x2000001d,
	0x4000001a, 0x60000019, 0x20000018, 0x00000011, 0xa000001b, 0x8000001d, 0xc0000019, 0xe000001e,
	0x00000020, 0x60000026, 0xe000002a, 0x80000024, 0x20000022, 0x40000023, 0xc000002c, 0xa000002d,
	0x80000022, 0x60000023, 0xa000002a, 0x40000020, 0xe0000025, 0x0000002a, 0xc000002e, 0x20000024,
	0x00000034, 0xe0000033, 0x20000032, 0xc0000038, 0x60000035, 0x80000034, 0x40000036, 0xa000003c,
	0x4000003d, 0x2000003c, 0xa000003b, 0xc000003a, 0x60000038, 0x00000036, 0x8000003a, 0xe000003c,
	0x8000004b, 0xa0000045, 0xe0000048, 0xc0000047, 0x6000004f, 0x4000004c, 0x00000047, 0x2000004e,
	0xc000004d, 0xa000004c, 0x2000004b, 0x4000004a, 0xe000004c, 0x8000004f, 0x00000046, 0x6000004d,
	0xc0000058, 0x20000052, 0xe0000056, 0x00000051, 0xa000005c, 0x40000056, 0x80000053, 0x60000052,
	0xc000005a, 0xe000005f, 0xa0000058, 0x80000053, 0x2000005b, 0x00000058, 0x40000059, 0x60000057,
};

u32 transformCurve(const u32 in, const u32 bits, const u32* lookupTable) {
	u64 x = _rotr64(in, 3 * bits);

	for (u32 i = 0; i < bits; ++i) {
		x = _rotl64(x, 3);
		x ^= lookupTable[u32(x)];
	}

	return x >> 29;
}

u32 mortonToHilbert3D(const u32 mortonIndex, const u32 bits) {
	return transformCurve(mortonIndex, bits, mortonToHilbertTable);
}

u32 Nimbus::PointCloud::hilbert3D(const vec3& point, const AABB& aabb) {
	const auto aabbMin = aabb.min();
	const auto aabbMax = aabb.max();
	float extent = aabb.extentLength();
	extent = (aabbMax[0] - aabbMin[0]) < extent ? extent : (aabbMax[0] - aabbMin[0]);
	extent = (aabbMax[1] - aabbMin[1]) < extent ? extent : (aabbMax[1] - aabbMin[1]);
	extent = (aabbMax[2] - aabbMin[2]) < extent ? extent : (aabbMax[2] - aabbMin[2]);
	const auto pos = (point - aabbMin) / extent;
	const u32 mortonVal = morton3D(pos.x, pos.y, pos.z);
	const u32 hilbertVal = mortonToHilbert3D(mortonVal, 11);
	return hilbertVal;
}

/**
 * @brief Performs sorting of the points in the point cloud using Hilbert Encoding.
 * @return The order of the points after the sorting.
 */
std::vector<u32> Nimbus::PointCloud::hilbertSorting() const {
	std::vector<std::pair<u32, u32>> orderAndHilbert(_position.size());
	const auto numPoints = _position.size();
	vec3 aabbMin = _aabb.min();
	vec3 aabbMax = _aabb.max();
	float extent = _aabb.extentLength();
	extent = (aabbMax[0] - aabbMin[0]) < extent ? extent : (aabbMax[0] - aabbMin[0]);
	extent = (aabbMax[1] - aabbMin[1]) < extent ? extent : (aabbMax[1] - aabbMin[1]);
	extent = (aabbMax[2] - aabbMin[2]) < extent ? extent : (aabbMax[2] - aabbMin[2]);
	for (int i = 0; i < numPoints; i++) {
		const auto pos = (_position[i] - aabbMin) / extent;
		const u32 mortonVal = morton3D(pos.x, pos.y, pos.z);
		const u32 hilbertVal = mortonToHilbert3D(mortonVal, 11);
		orderAndHilbert[i] = std::make_pair(i, hilbertVal);
	}

	std::sort(std::execution::par_unseq, orderAndHilbert.begin(), orderAndHilbert.end(), compareFunction);

	std::vector<u32> order(_position.size());
	for (int i = 0; i < numPoints; i++) {
		order[i] = orderAndHilbert[i].first;
	}
	return order;
}

/**
 * @brief Performs sorting of the points in the point cloud using Morton Encoding.
 * @return The order of the points after the sorting.
 */
std::vector<u32> Nimbus::PointCloud::mortonSorting() const {
	std::vector<std::pair<u32, u32>> orderAndMorton(_position.size());
	const auto numPoints = _position.size();
	vec3 aabbMin = _aabb.min();
	vec3 aabbMax = _aabb.max();
	float extent = _aabb.extentLength();
	extent = (aabbMax[0] - aabbMin[0]) < extent ? extent : (aabbMax[0] - aabbMin[0]);
	extent = (aabbMax[1] - aabbMin[1]) < extent ? extent : (aabbMax[1] - aabbMin[1]);
	extent = (aabbMax[2] - aabbMin[2]) < extent ? extent : (aabbMax[2] - aabbMin[2]);
	for (int i = 0; i < numPoints; i++) {
		const auto pos = (_position[i] - aabbMin) / extent;
		const u32 mortonVal = morton3D(pos.x, pos.y, pos.z);
		orderAndMorton[i] = std::make_pair(i, mortonVal);
	}

	std::sort(std::execution::par_unseq, orderAndMorton.begin(), orderAndMorton.end(), compareFunction);

	std::vector<u32> order(_position.size());
	for (int i = 0; i < numPoints; i++) {
		order[i] = orderAndMorton[i].first;
	}
	return order;
}

std::vector<u32> Nimbus::PointCloud::kdTreeSorting() {
	const auto numPoints = _position.size();
	auto numMeshlets = static_cast<u32>(_position.size() / _meshletSize);
	if (numPoints > numMeshlets * _meshletSize)
		numMeshlets++;
	using PC2KD = PointCloudAdaptor<PointCloud>;
	using kdTree = nanoflann::KDTreeSingleIndexDynamicAdaptor<
		nanoflann::L2_Simple_Adaptor<float, PC2KD>, PC2KD, 3 /* dim */
	>;
	const PC2KD pc2kd(this);  // The adaptor
	kdTree tree(3, pc2kd, { _meshletSize, nanoflann::KDTreeSingleIndexAdaptorFlags::None, 0 });
	std::vector<size_t>                         ret_index(_meshletSize);
	std::vector<float>                         out_dist_sqr(_meshletSize);
	nanoflann::KNNResultSet<float> resultSet(_meshletSize);
	std::vector<size_t> neighborsOrder(numMeshlets);
	std::iota(neighborsOrder.begin(), neighborsOrder.end(), 0);
	std::ranges::shuffle(neighborsOrder, std::default_random_engine(numPoints));
	std::vector<u32> order(_position.size());
	for (int i = 0; i < numMeshlets; ++i) {
		resultSet.init(ret_index.data(), out_dist_sqr.data());
		tree.findNeighbors(resultSet, _position[neighborsOrder[i] * _meshletSize].data.data, { 0.0f, true });
		u32 limit = _meshletSize;
		const u32 left = numPoints - i * _meshletSize;
		if (left < limit)
			limit = left;
		for (int j = 0; j < limit; ++j) {
			order[i * _meshletSize + j] = ret_index[j];
			tree.removePoint(ret_index[j]);
		}
	}
	return order;
}

template<typename T>
void reorder(std::vector<u32>& order, std::vector<T>& v) {
	std::vector<T> temp(v.size());
	for (size_t i = 0; i < v.size(); ++i) {
		temp[i] = v[order[i]];
	}
	v = std::move(temp);
}

/**
 * \brief Optimiza el orden de los puntos en la estructura de datos para mejorar la eficiencia del renderizado.
 * \param sortMethod Metodo de ordenacion a utilizar.
 * \param saveMeshlets Indica si se deben guardar los meshlets en disco.
 * \param shuffleMeshlet Indica si se deben mezclar los puntos dentro de cada meshlet.
 */
void Nimbus::PointCloud::spatialOrdering(const SortingMethod sortMethod, const bool saveMeshlets, const bool shuffleMeshlet) {
	try {
		_optimized = false;

		_meshletSize = computeMeshletSize(getNumberOfPoints());
		_numPoints = getNumberOfPoints();

		fmt::print("Using {} as meshlet size\n", _meshletSize);

		const auto numPoints = _position.size();
		auto numMeshlets = static_cast<u32>(_position.size() / _meshletSize);
		if (numPoints > static_cast<u64>(numMeshlets) * _meshletSize)
			numMeshlets++;
		fmt::print("{} meshlets\n", numMeshlets);

		std::vector<u32> order;

		ChronoUtilities::initChrono();

		switch (sortMethod) {
		case SortingMethod::Hilbert:
			order = hilbertSorting();
			break;
		case SortingMethod::Morton:
			order = mortonSorting();
			break;
		case SortingMethod::kDTree:
			order = kdTreeSorting();
			break;
		}


		auto timeMs = ChronoUtilities::getDuration();
		spdlog::info("Point cloud spatial ordering of {} points in {} ms", _position.size(), timeMs);

		ChronoUtilities::initChrono();
		_meshlets.clear();
		_meshlets.reserve(numMeshlets);
		std::vector<vec3> newPoints(_position.size());
		for (u32 i = 0; i < numMeshlets; ++i) {
			u32 limit = _meshletSize;
			const u32 left = numPoints - i * _meshletSize;
			if (left < limit)
				limit = left;
			if (shuffleMeshlet)
				std::shuffle(order.begin() + i * _meshletSize, order.begin() + i * _meshletSize + limit, std::default_random_engine(numPoints));
			_meshlets.emplace_back();
			for (u32 j = 0; j < limit; ++j) {
				const u32 pointIndex = order[i * _meshletSize + j];
				newPoints[i * _meshletSize + j] = _position[pointIndex];
				_meshlets.back().aabb.update(_position[pointIndex]);
			}
		}

		_position = std::move(newPoints);

		if (_rgbColor.size() == order.size()) {
			reorder(order, _rgbColor);
		} else {
			spdlog::warn("{} doesn't have a color for each point", this->_name);
		}

		if (_normals.size() == order.size()) {
			reorder(order, _normals);
		} else if (!_normals.empty()) {
			spdlog::warn("{} doesn't have a normal for each point", this->_name);
		}

		timeMs = ChronoUtilities::getDuration();
		spdlog::info("Point cloud reordering of {} points in {} ms", _position.size(), timeMs);

		ChronoUtilities::initChrono();
		switch (sortMethod) {
		case SortingMethod::Hilbert:
			computeMetrics("Hilbert");
			break;
		case SortingMethod::Morton:
			computeMetrics("Morton");
			break;
		case SortingMethod::kDTree:
			computeMetrics("kDTree");
			break;
		}

		timeMs = ChronoUtilities::getDuration();
		spdlog::info("Point cloud metrics computation of {} points in {} ms", _position.size(), timeMs);

		if (saveMeshlets) {
			saveBinaryFile(_savePath);
			initializeReadStreams();
		}
		_position.clear();
		_rgbColor.clear();

		_optimized = true;
		_meshletsNeedUpdate = true;
		needUpdate();
		_optimized = true;
	} catch (std::exception& e) {
		throw e;
	}
}

void Nimbus::PointCloud::computeMetrics(const std::string& methodName) {
	double meanExtent = 0, stdDesv = 0, minExtent = FLT_MAX, maxExtent = -FLT_MAX, meanSquareness = 0;

	for (auto& meshlet : _meshlets) {
		double extentLength = meshlet.aabb.extentLength();
		meanExtent += extentLength;
		minExtent = std::min(minExtent, extentLength);
		maxExtent = std::max(maxExtent, extentLength);
		const vec3 size = meshlet.aabb.size();
		double squareness = abs(size.x - size.y);
		squareness += abs(size.x - size.z);
		squareness += abs(size.y - size.z);
		squareness /= 3;
		meanSquareness += squareness;
	}
	const auto numMeshlets = _meshlets.size();
	meanExtent /= numMeshlets;
	_meanMeshletExtent = meanExtent;
	meanSquareness /= numMeshlets;
	for (auto& meshlet : _meshlets) {
		const auto diff = meshlet.aabb.extentLength() - meanExtent;
		stdDesv += diff * diff;
	}
	stdDesv /= numMeshlets;
	stdDesv = std::sqrt(stdDesv);
	spdlog::info("Spatial sorting statistics for {}", this->_name);
	spdlog::info("{}\n", methodName);
	spdlog::info("Min extent: {}", minExtent);
	spdlog::info("Max extent: {}", maxExtent);
	spdlog::info("Mean extent: {}", meanExtent);
	spdlog::info("Std desv: {}", stdDesv);
	spdlog::info("Mean squareness: {}\n", meanSquareness);
}

u32 Nimbus::PointCloud::computeMeshletSize(const u64 numPoints) {
	u32 meshletSize = std::pow(2, std::ceil(log2f(numPoints / 100000.f)));
	meshletSize = std::min(meshletSize, (u32)UINT16_MAX);
	meshletSize = std::max(meshletSize, 512u);
	return meshletSize;
}

void Nimbus::PointCloud::recomputeMeshlets()
{
	u32 meshletSize = computeMeshletSize(_numPoints);
	std::ifstream file(_savePath + ".NimbusCloudPos", std::ios::binary);
	if (file.good()) {
		std::vector<vec3> meshletPoints;
		meshletPoints.resize(meshletSize);
		i64 remainingPoints = _numPoints;
		u32 currentMeshlet = 0;
		while (remainingPoints > 0) {
			file.read((char*)meshletPoints.data(), meshletSize * sizeof(vec3));
			AABB aabb;
			for (auto& point : meshletPoints) {
				aabb.update(point);
			}
			_meshlets[currentMeshlet++].aabb = aabb;
			remainingPoints -= meshletSize;
		}
	}
	file.close();
	saveMetadata(_savePath);
}

//======================== Add/Set methods =========================================

/**
 * A�ade una nueva subnube. Se transfiere la gestion del puntero a la nube desde el momento en que se ejecute esta funcion.
 *
 * \param newSubcloud nueva sub nube a a�adir.
 */
void Nimbus::PointCloud::addSubcloud(std::unique_ptr<PointCloud> newSubcloud) {
	if (newSubcloud->getNumberOfPoints() > 0)
		_aabb.update(newSubcloud->_aabb);
	_subClouds.push_back(std::move(newSubcloud));
}

/**
 * A�ade un nuevo punto a la nube. Si se van a a�adir varios puntos, es preferible usar addNewPoints.
 *
 * \param newPoint nuevo punto a a�adir.
 */
void Nimbus::PointCloud::addNewPoint(const VAO::Point& newPoint) {
	_position.push_back(newPoint._point);
	_rgbColor.push_back(newPoint._rgb);
	_aabb.update(newPoint._point);
}

/**
 * A�ade nuevos puntos al final de la nube.
 *
 * \param newPoints vector de puntos a a�adir.
 */
void Nimbus::PointCloud::addNewPoints(const std::vector<VAO::Point>& newPoints) {
	_position.reserve(_position.size() + newPoints.size());
	for (const auto& p : newPoints) {
		_position.push_back(p._point);
		_rgbColor.push_back(p._rgb);
		_aabb.update(p._point);
	}
	//spatialOrdering();
}

/**
 * \brief A�ade nuevas normales al final del vector de normales.
 * \param normals nuevas normales a a�adir.
 */
void Nimbus::PointCloud::addNormals(const std::vector<vec3>& normals) {
	_normals.reserve(_normals.size() + normals.size());
	for (const auto& p : _normals) {
		_normals.push_back(p);
	}
}


/**
 * \brief Sustituye los puntos actuales de la nube por los especificados en el vector.
 * \param newPoints Nuevos puntos
 * \param optimizePointOrder Indica si se quiere que se optimice el orden de los puntos para el renderizado. Debe ser "false" si más tarde se añadirán datos asociados a cada punto.
 */
void Nimbus::PointCloud::setNewPoints(const std::vector<VAO::Point>& newPoints, const bool optimizePointOrder) {
	_position.clear();
	_rgbColor.clear();
	_position.reserve(newPoints.size());
	_rgbColor.reserve(newPoints.size());
	for (const auto& p : newPoints) {
		_position.push_back(p._point);
		_rgbColor.push_back(p._rgb);
		_aabb.update(p._point);
	}
	if (optimizePointOrder)
		spatialOrdering(SortingMethod::Hilbert);
}


/**
 * \brief Sustituye las normales de los puntos de la nube por los especificados en en el vector.
 * \param newNormals Nuevas normales
 */
void Nimbus::PointCloud::setNormals(const std::vector<vec3>& newNormals) {
	_normals.clear();
	_normals.reserve(newNormals.size());
	for (const auto& p : newNormals) {
		_normals.push_back(p);
	}
}

/**
 * \brief Sustituye el vector actual del color indicado por el nuevo.
 * \param colorVector nuevo vector de colores.
 */
void Nimbus::PointCloud::setColor(const std::vector<vec3>& colorVector) {
	if (colorVector.size() != _position.size())
		throw std::out_of_range("[Nimbus::PointCloud::setColor]->El tamaño del vector de colores RGB y el tamaño de la nube de puntos debe de ser el mismo");
	_rgbColor.resize(colorVector.size());
	int i = 0;
	for (auto& color : _rgbColor) {
		color = VAO::Point::get8BitRGBColor(colorVector[i++]);
	}

}


/**
 * \brief Sustituye el vector actual del color indicado por el nuevo.
 * \param colorVector nuevo vector de colores.
 */
void Nimbus::PointCloud::setColor(const std::vector<vec4>& colorVector) {
	if (colorVector.size() != _position.size())
		throw std::out_of_range("[Nimbus::PointCloud::setColor]->El tamaño del vector de colores RGB y el tamaño de la nube de puntos debe de ser el mismo");
	_rgbColor.resize(colorVector.size());
	int i = 0;
	for (auto& color : _rgbColor) {
		color = VAO::Point::get8BitRGBColor(colorVector[i++]);
	}
}

/**
 * \brief Sustituye el vector de datos especificado por el nuevo
 * \param att Tipo de vector de datos
 * \param band Banda del tipo de dato.
 * \param dataVector Vector con los datos
 */
void Nimbus::PointCloud::setData(const Attribute& att, const u32 band, const std::vector<float>& dataVector) {
	if (att != Attribute::RGB) {
		u8 dataType = (u8)att;
		if (_fusionData[dataType].empty()) {
			_fusionData[dataType].resize(maxBandsPerAtt[dataType]);
			_fusionDataMinMax[dataType].resize(maxBandsPerAtt[dataType]);
		} else
			_fusionData[dataType][band].clear();
		_fusionData[dataType][band] = dataVector;
		auto minMaxElements = std::ranges::minmax_element(_fusionData[dataType][band]);
		_fusionDataMinMax[dataType][band].x = *minMaxElements.min;
		_fusionDataMinMax[dataType][band].y = *minMaxElements.max;
		const vec2& minMax = _fusionDataMinMax[dataType][band];
		const f32 range = minMax.y - minMax.x;
		for (auto& value : _fusionData[dataType][band]) {
			value = (value - minMax.x) / range;
		}
		_fusionData[dataType][band].resize(getNumberOfPoints());
		spdlog::info("{}_Band{}-> Min: {} - Max: {}", AttributeToString(att).c_str(), band, minMax.x, minMax.y);
		std::string tmp = std::string(_renderAttributes[dataType]).substr(0, 3);
		const std::string fileExtension = std::format(".NimbusCloud{}", tmp);
		std::ofstream of;
		if (std::filesystem::exists(_savePath + fileExtension)) {
			of.open(_savePath + fileExtension, std::ios::in | std::ios::out | std::ios::binary);
			of.seekp((std::streamoff)band * getNumberOfPoints() * sizeof(f32));
		} else {
			of.open(_savePath + fileExtension, std::ios::out | std::ios::binary);
		}
		saveFusionData(of, (Attribute)dataType, band);
		if ((band + 1) > _availableDataBands[dataType])
			_availableDataBands[dataType] = (band + 1);
		saveMetadata(_savePath);
		_fusionData[dataType][band].clear();
	} else {
		throw std::invalid_argument("[Nimbus::PointCloud::setData]->No se puede modificar el color RGB de la nube de puntos con esta función. Utilice setColor.");
	}
	//needUpdate(dataType);
}


//======================== Get methods =========================================

/**
 * \brief Look for the specified subcloud and return it if found.
 * \param subcloudName cloud name to look for.
 * \return pointer to the subcloud if found, nullptr otherwise.
 */
Nimbus::PointCloud* Nimbus::PointCloud::getSubcloud(const std::string& subcloudName) const {
	for (const auto& subcloud : _subClouds) {
		if (subcloud->getName() == subcloudName) {
			return subcloud.get();
		}
	}
	return nullptr;
}

/**
 * \brief return all the subclouds in this cloud. If there isn't any subcloud, the vector will be empty.
 * \return a vector containing all subclouds in this cloud.
 */
const std::vector<std::unique_ptr<Nimbus::PointCloud>>& Nimbus::PointCloud::getSubclouds() {
	return _subClouds;
}

std::vector<vec3>& Nimbus::PointCloud::getNormals() {
	return _normals;
}

/**
 * Devuelve un vector con todos los puntos asociados a esta nube.
 */
std::vector<vec3> Nimbus::PointCloud::getPointsPosition() {
	loadPositionsFromBinaryFile(_savePath);
	return std::move(_position);
}

std::vector<u32> Nimbus::PointCloud::getPointsColor() {
	loadRGBFromBinaryFile(_savePath);
	return std::move(_rgbColor);
}

const std::vector<Nimbus::PointCloud::Meshlet>& Nimbus::PointCloud::getMeshlets() {
	return _meshlets;
}

u32 Nimbus::PointCloud::getMeshletSize() const {
	return _meshletSize;
}

size_t Nimbus::PointCloud::getMeshletNumber() const {
	if (!_optimized)
		return 0;
	return _meshlets.size();
}

u64 Nimbus::PointCloud::getNumberOfPoints() const {
	if (_position.empty())
		return _numPoints;
	return _position.size();
}

std::string Nimbus::PointCloud::getCloudSummary() const {
	std::string sum = "Point number: " + std::to_string(getNumberOfPoints());
	return sum;
}

bool Nimbus::PointCloud::isEnabled() const {
	return _enabled;
}

bool Nimbus::PointCloud::isControlledByRenderer() const {
	return _controlledByRenderer;
}

bool Nimbus::PointCloud::haveNormals() const {
	return !_normals.empty();
}

/**
 * \brief Cambia a la banda especificada del atributo indicado.
 * \param att Atributo.
 * \param band Banda del atributo.
 */
void Nimbus::PointCloud::switchAttribute(const Attribute& att, const u16& band) {
	_controlledByRenderer = false;
	_attToUse = att;
	_attributeBand = band;
	reloadAttribute();
}

/**
 * Request the data associated with the specified attribute and band.
 * @param meshletIndex Meshlet index to load.
 * @param numData Meshlet data to load. If 0, all meshlet data will be loaded.
 */
void Nimbus::PointCloud::requestMeshletData(const u32 meshletIndex, u16 numData) {
	_controlledByRenderer = false;
	if (numData == 0)
		numData = _meshletSize;
	_meshlets[meshletIndex].pointsRequired = numData;
}

/**
 * Unload the data associated with the specified meshlet.
 * @param meshletIndex Meshlet index to unload.*/
void Nimbus::PointCloud::unloadMeshletData(const u32 meshletIndex) {
	_meshlets[meshletIndex].pointsRequired = 0;
}

/**
 * \brief
 */
void Nimbus::PointCloud::returnControlToRenderer() {
	_controlledByRenderer = true;
}

vec3 Nimbus::PointCloud::getPosition(u32 pointIndex) {
	_controlledByRenderer = false;
	u32 meshletIndex = pointIndex / _meshletSize;
	u16 pointInsideMeshlet = pointIndex % _meshletSize;
	if (_meshlets[meshletIndex].pointsLoaded <= pointInsideMeshlet)
		requestMeshletData(meshletIndex, _meshletSize);
	while (_meshlets[meshletIndex].pointsLoaded <= pointInsideMeshlet) {
		std::this_thread::yield();
	}
	return _meshlets[meshletIndex].pos[pointInsideMeshlet];
}

std::vector<vec3> Nimbus::PointCloud::getPositions(u32 offset, u32 numPoints) const {
	std::ifstream rf;
	std::string fileExtension = ".NimbusCloudPos";
	rf.open(_savePath + fileExtension, std::ios::in | std::ios::binary);
	std::vector<vec3> data(numPoints);
	if (rf.is_open()) {
		rf.seekg(offset * sizeof(vec3));
		rf.read((char*)data.data(), numPoints * sizeof(vec3));
		//if (rf.fail())
		//	spdlog::error("Error reading attribute data from file [Band {}]", band);
	}
	return std::move(data);
}


//======================== Update OpenGL Resources methods =========================================

/**
 * \brief Obliga a actualizar los recursos OpenGL de esta nube en la siguiente llamada de dibujado.
 */
void Nimbus::PointCloud::needUpdate() {
	_meshletsNeedUpdate = true;
	_pickedNeedUpdate = true;
}

/**
 * \brief Actualiza los recursos OpenGL asociados a esta nube. Esta funcion solo se puede llamar desde el hilo principal.
 *
 */
void Nimbus::PointCloud::updateCloud() {
	reloadAttribute();

	if (_meshletsNeedUpdate && !_meshlets.empty()) {
		std::vector<AABB> meshletsAABBs(_meshlets.size());
		for (int i = 0; i < _meshlets.size(); ++i) {
			meshletsAABBs[i] = _meshlets[i].aabb;
		}
		_currentFrameCulling.resize(_meshlets.size());
		_lastFrameCulling.resize(_meshlets.size());
		_GPUResources.reserveGPUMappedMemory<u16>(GPUResources::SSBOSlots::CullingBuffer, _meshlets.size(), GL_MAP_READ_BIT);
		_GPUResources.reserveGPUMappedMemory<CompactInfo>(GPUResources::SSBOSlots::CompactionInfo, _meshlets.size(), GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);
		_GPUResources.uploadToGPU(GPUResources::SSBOSlots::Meshlets, meshletsAABBs.data(), meshletsAABBs.size(), 0);
		_meshletsNeedUpdate = false;
	}

	if (_pickedNeedUpdate) {
		_GPUResources.uploadToGPU(GPUResources::SSBOSlots::PickedPoint, _picked.data(), _picked.size());
		_pickedNeedUpdate = false;
	}
}


void Nimbus::PointCloud::reloadAttribute() {
	if (_attToUse != _prevAttToUse) {
		_prevAttToUse = _attToUse;
		_attributeBand = 0;

		std::ranges::fill(_lastFrameCulling, 0);
		for (auto& meshlet : _meshlets) {
			meshlet.pointsRequired = 0;
			meshlet.pointsLoaded = 0;
		}
		for (auto& rf : _attRf) {
			reopenAttributeReadStream(rf);
		}
	} else if (_attributeBand != _prevAttributeBand) {
		_attributeBand = std::min(_attributeBand, _availableDataBands[(u8)_attToUse] - 1);
		std::ranges::fill(_lastFrameCulling, 0);
		for (auto& meshlet : _meshlets) {
			meshlet.pointsRequired = 0;
			meshlet.pointsLoaded = 0;
		}
	}
	_prevAttributeBand = _attributeBand;
}

//======================== IO methods =========================================

void Nimbus::PointCloud::initializeReadStreams() {
	const std::string posFilename = std::format("{}.NimbusCloudPos", _savePath);
	const std::string rgbFilename = std::format("{}.NimbusCloudRGB", _savePath);
	constexpr u8 maxThreadNumber = MemoryManager::_numThreads;
	_posRf.resize(maxThreadNumber);
	_attRf.resize(maxThreadNumber);

	for (int i = 0; i < maxThreadNumber; ++i) {
		_posRf[i].rdbuf()->pubsetbuf(nullptr, 0);
		_posRf[i].open(posFilename, std::ios::in | std::ios::binary);
		_attRf[i].rdbuf()->pubsetbuf(nullptr, 0);
		_attRf[i].open(rgbFilename, std::ios::in | std::ios::binary);

		if (!_posRf[i].good() || !_attRf[i].good()) {
			spdlog::error("Filestream creation failed\n");
		}
	}
}

void Nimbus::PointCloud::reopenAttributeReadStream(std::ifstream& rf) {
	std::string tmp = std::string(_renderAttributes[(u8)_attToUse]).substr(0, 3);
	const std::string fileExtension = std::format(".NimbusCloud{}", tmp);
	rf.close();
	rf.clear();
	rf.open(_savePath + fileExtension, std::ios::in | std::ios::binary);
}

void Nimbus::PointCloud::closeReadStreams() {
	constexpr u8 maxThreadNumber = MemoryManager::_numThreads;
	for (int i = 0; i < maxThreadNumber; ++i) {
		_posRf[i].close();
		_attRf[i].close();
	}
}

void Nimbus::PointCloud::loadMeshletPosition(std::ifstream& rf, const u32 meshletIndex, const u16 numPointsToLoad, const u16 insideMeshletOffset) {
	if (meshletIndex != _meshlets.size() - 1) {
		const std::streamoff baseIndex = meshletIndex * _meshletSize + insideMeshletOffset;
		rf.seekg(baseIndex * sizeof(vec3));
		if (_meshlets[meshletIndex].pos.size() >= numPointsToLoad + insideMeshletOffset) {
			rf.read((char*)&_meshlets[meshletIndex].pos[insideMeshletOffset], sizeof(vec3) * numPointsToLoad);
			if (!rf.good()) {
				rf.close();
				rf.clear();
				rf.open(_savePath + "NimbusCloudPos", std::ios::in | std::ios::binary);
			}
		}
	}
}


void Nimbus::PointCloud::loadMeshletAttribute(std::ifstream& rf, const u32 meshletIndex, const u16 numPointsToLoad, const u16 insideMeshletOffset) {
	if (meshletIndex != _meshlets.size() - 1) {
		const std::streamoff baseIndex = meshletIndex * _meshletSize + insideMeshletOffset + _attributeBand * _numPoints;
		rf.seekg(baseIndex * sizeof(u32));
		if (_meshlets[meshletIndex].att.size() >= numPointsToLoad + insideMeshletOffset) {
			rf.read((char*)&_meshlets[meshletIndex].att[insideMeshletOffset], sizeof(u32) * numPointsToLoad);
			if (!rf.good()) {
				reopenAttributeReadStream(rf);
			}
		}
	}
}


void Nimbus::PointCloud::saveMetadata(const std::string& path) {
	std::ofstream wf(path + ".NimbusCloud", std::ios::out | std::ios::binary);
	if (wf.is_open()) {
		constexpr char version[6] = "1.0.2";
		wf.write(version, sizeof(version));
		auto nameLength = _name.length();
		wf.write(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
		wf.write(_name.data(), nameLength);

		wf.write(reinterpret_cast<char*>(&_pos), sizeof(dvec3));
		wf.write(reinterpret_cast<char*>(&_rot), sizeof(dvec3));
		wf.write(reinterpret_cast<char*>(&_scale), sizeof(dvec3));
		wf.write(reinterpret_cast<char*>(&_offset), sizeof(dvec3));
		wf.write(reinterpret_cast<char*>(&_aabb), sizeof(AABB));

		u64 numPoints = getNumberOfPoints();
		wf.write((char*)&numPoints, sizeof(numPoints));
		/*u8 numDataSources = NUM_DATA_SOURCES;
		wf.write((char*)numDataSources, sizeof(u8));*/

		for (auto& databands : _availableDataBands) {
			wf.write((char*)&databands, sizeof(databands));
		}
		int i = 0;
		for (auto& minMax : _fusionDataMinMax) {
			if (!minMax.empty())
				wf.write((char*)minMax.data(), sizeof(vec2) * _availableDataBands[i]);
			i++;
		}

		wf.write(reinterpret_cast<char*>(&_meshletSize), sizeof(u32));
		union {
			float f;
			u32 binary;
		} floatToBinary;
		floatToBinary.f = _meanMeshletExtent;
		wf.write(reinterpret_cast<char*>(&floatToBinary.binary), sizeof(u32));
		auto numMeshlets = _meshlets.size();
		wf.write((char*)&numMeshlets, sizeof(size_t));
		for (auto& meshlet : _meshlets) {
			wf.write((char*)&meshlet.aabb, sizeof(AABB));
		}
		wf.close();
	} else {
		throw std::runtime_error("Nimbus::PointCloud::saveMetadata -> Cannot open file: " + path);
	}
}

void Nimbus::PointCloud::savePosition(std::ofstream& wf) {
	wf.write((char*)_position.data(), sizeof(vec3) * _position.size());
}

void Nimbus::PointCloud::saveRGB(std::ofstream& wf) {
	wf.write((char*)_rgbColor.data(), sizeof(u32) * _rgbColor.size());
}

void Nimbus::PointCloud::saveFusionData(std::ofstream& wf, const Attribute dataSource, const u32 band) {
	wf.write((char*)_fusionData[(u8)dataSource][band].data(), sizeof(float) * _fusionData[(u8)dataSource][band].size());
}

void Nimbus::PointCloud::saveBinaryFile(const std::string& path) {
	const auto start = std::chrono::high_resolution_clock::now();

	saveMetadata(path);

	auto numMeshlets = _meshlets.size();
	std::thread posSaveThread([path, numMeshlets, this]() {
		std::string filename = std::format("{}.NimbusCloudPos", path);
		std::ofstream positionWf(filename, std::ios::out | std::ios::binary);
		if (positionWf.is_open()) {
			savePosition(positionWf);
			positionWf.close();
		} else {
			spdlog::error("Couldnt open {} to save meshlet position", filename);
		}
	});

	std::thread rgbSaveThread([path, numMeshlets, this]() {
		std::string filename = std::format("{}.NimbusCloudRGB", path);
		std::ofstream rgbWf(filename, std::ios::out | std::ios::binary);
		if (rgbWf.is_open()) {
			saveRGB(rgbWf);
			rgbWf.close();
		} else {
			spdlog::error("Couldnt open {} to save meshlet RGB", filename);
		}
	});

	posSaveThread.join();
	rgbSaveThread.join();
	const auto end = std::chrono::high_resolution_clock::now();
	const auto int_s = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	spdlog::info("Save in {} ms", int_s.count());
}

void Nimbus::PointCloud::loadBinaryFile(const std::string& path) {
	_loading = true;
	_enabled = false;
	const auto start = std::chrono::high_resolution_clock::now();
	_savePath = path;
	std::ranges::fill(_availableDataBands, 0);
	std::ifstream rf;
	rf.open(path + ".NimbusCloud", std::ios::in | std::ios::binary);
	if (rf.is_open()) {
		spdlog::info("Loading started");
		char version[6] = "1.0.2";
		//spdlog::info("Binary file v{}", version);
		rf.read(version, sizeof(version));
		if (strcmp(version, "1.0.2") != 0){
			rf.close();
			_optimized = false;
			_enabled = false;
			_loading = false;
			throw std::runtime_error(fmt::format("Nimbus::PointCloud::loadBinaryFile -> Metada file version mismatch ({} expected, {} found)","1.0.2", std::string(version)));
		}
		size_t nameLength;
		rf.read(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
		_name.resize(nameLength);
		rf.read(_name.data(), nameLength);

		rf.read(reinterpret_cast<char*>(&_pos), sizeof(dvec3));
		rf.read(reinterpret_cast<char*>(&_rot), sizeof(dvec3));
		rf.read(reinterpret_cast<char*>(&_scale), sizeof(dvec3));
		rf.read(reinterpret_cast<char*>(&_offset), sizeof(dvec3));
		rf.read(reinterpret_cast<char*>(&_aabb), sizeof(AABB));
		rf.read(reinterpret_cast<char*>(&_numPoints), sizeof(_numPoints));

		for (auto& databands : _availableDataBands)
			rf.read((char*)&databands, sizeof(databands));
		int i = 0;
		for (auto& minMaxVec : _fusionDataMinMax) {
			minMaxVec.resize(_availableDataBands[i++]);
			for (auto& minMax : minMaxVec)
				rf.read((char*)&minMax, sizeof(vec2));
		}

		rf.read(reinterpret_cast<char*>(&_meshletSize), sizeof(u32));
		union {
			float f;
			u32 binary;
		} floatToBinary;
		floatToBinary.f = _meanMeshletExtent;
		rf.read(reinterpret_cast<char*>(&floatToBinary.binary), sizeof(u32));
		_meanMeshletExtent = floatToBinary.f;
		size_t numMeshlets;
		rf.read((char*)&numMeshlets, sizeof(size_t));
		_meshlets.resize(numMeshlets);
		for (auto& meshlet : _meshlets) {
			rf.read((char*)&meshlet.aabb, sizeof(AABB));
		}
		rf.close();

		initializeReadStreams();

		spdlog::info("Loading finished");
		needUpdate();
	} else {
		throw std::runtime_error("Nimbus::PointCloud::loadBinaryFile -> Cannot open file");
	}
	const auto end = std::chrono::high_resolution_clock::now();
	const auto int_s = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	spdlog::info("Load in {} ms", int_s.count());
	_optimized = true;
	_enabled = true;
	_loading = false;
}

void Nimbus::PointCloud::loadPositionsFromBinaryFile(const std::string& path) {
	std::ifstream rf;
	rf.open(path + ".NimbusCloudPos", std::ios::in | std::ios::binary);
	if (rf.is_open()) {
		rf.seekg(0, std::ifstream::end);
		const auto size = rf.tellg();
		rf.seekg(0, std::ifstream::beg);
		const auto numPoints = size / sizeof(vec3);
		_position.resize(numPoints);
		rf.read((char*)_position.data(), size);
	}
}

void Nimbus::PointCloud::loadRGBFromBinaryFile(const std::string& path) {
	std::ifstream rf;
	rf.open(path + ".NimbusCloudRGB", std::ios::in | std::ios::binary);
	if (rf.is_open()) {
		rf.seekg(0, std::ifstream::end);
		const auto size = rf.tellg();
		rf.seekg(0, std::ifstream::beg);
		const auto numColors = size / sizeof(u32);
		_rgbColor.resize(numColors);
		rf.read((char*)_rgbColor.data(), size);
	}
}


//======================== Utility methods =========================================

void Nimbus::PointCloud::clearData() {
	_position.clear();
	_rgbColor.clear();
	_normals.clear();
	for (auto& fusionData : _fusionData) {
		fusionData.clear();
	}
	needUpdate();
	spdlog::info("{} data cleared", _name);
}

std::string Nimbus::PointCloud::AttributeToString(const Attribute& dataSource) {
	switch (dataSource) {
	case Attribute::RGB:
		return "RGB";
	default:
		return "None";
	}
}

void Nimbus::PointCloud::updateAABB(const AABB& aabb) {
	_aabb.update(aabb);
}

void Nimbus::PointCloud::updateAABB(const vec3& point) {
	_aabb.update(point);
}

Nimbus::Attribute Nimbus::PointCloud::getCurrentAttribute() {
	return _attToUse;
}
