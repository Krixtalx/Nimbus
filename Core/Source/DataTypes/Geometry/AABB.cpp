#include "CorePch.h"
#include "AABB.h"


AABB::AABB(const dvec3& min, const dvec3& max) :
	_min(min), _max(max) {
}

AABB::AABB(const AABB& aabb) = default;

AABB::~AABB() = default;

AABB& AABB::operator=(const AABB& aabb) = default;

AABB AABB::dot(const mat4& matrix) const {
	return { matrix * vec4(_min, 1.0f), matrix * vec4(_max, 1.0f) };
}

std::vector<dvec3> AABB::corners() const {
	std::vector<dvec3> corners(8);
	corners[0] = _min;
	corners[1] = { _min.x, _min.y, _max.z };
	corners[2] = { _min.x, _max.y, _min.z };
	corners[3] = { _min.x, _max.y, _max.z };
	corners[4] = { _max.x, _min.y, _min.z };
	corners[5] = { _max.x, _min.y, _max.z };
	corners[6] = { _max.x, _max.y, _min.z };
	corners[7] = _max;
	return corners;
}

void AABB::update(const AABB& aabb) {
	this->update(aabb.max());
	this->update(aabb.min());
}

void AABB::update(const vec3& point) {
	if (point.x < _min.x) { _min.x = point.x; }
	if (point.y < _min.y) { _min.y = point.y; }
	if (point.z < _min.z) { _min.z = point.z; }

	if (point.x > _max.x) { _max.x = point.x; }
	if (point.y > _max.y) { _max.y = point.y; }
	if (point.z > _max.z) { _max.z = point.z; }
}

void AABB::update(const dvec3& point) {
	if (point.x < _min.x) { _min.x = point.x; }
	if (point.y < _min.y) { _min.y = point.y; }
	if (point.z < _min.z) { _min.z = point.z; }

	if (point.x > _max.x) { _max.x = point.x; }
	if (point.y > _max.y) { _max.y = point.y; }
	if (point.z > _max.z) { _max.z = point.z; }
}

bool AABB::operator!=(const AABB& aabb)
{
	return glm::length(aabb.min() - vec3(_min)) < glm::epsilon<float>() && glm::length(aabb.max() - vec3(_max)) < glm::epsilon<float>();
}

bool AABB::rayIntersects(const vec3& origin, const vec3& direction) const { // https://web.archive.org/web/20090803054252/http://tog.acm.org/resources/GraphicsGems/gems/RayBox.c
	static constexpr unsigned NUMDIM = 3;
	static constexpr unsigned RIGHT = 0;
	static constexpr unsigned LEFT = 1;
	static constexpr unsigned MIDDLE = 2;

	double coord[NUMDIM];		/*ray */
	{
		char inside = TRUE;
		char quadrant[NUMDIM];
		int i;
		double maxT[NUMDIM];
		double candidatePlane[NUMDIM];

		/* Find candidate planes; this loop can be avoided if
		rays cast all from the eye(assume perpsective view) */
		for (i = 0; i < NUMDIM; i++)
			if (origin[i] < _min[i]) {
				quadrant[i] = LEFT;
				candidatePlane[i] = _min[i];
				inside = FALSE;
			} else if (origin[i] > _max[i]) {
				quadrant[i] = RIGHT;
				candidatePlane[i] = _max[i];
				inside = FALSE;
			} else {
				quadrant[i] = MIDDLE;
			}

			/* Ray origin inside bounding box */
			if (inside) {
				return true;
			}


			/* Calculate T distances to candidate planes */
			for (i = 0; i < NUMDIM; i++)
				if (quadrant[i] != MIDDLE && direction[i] != 0.)
					maxT[i] = (candidatePlane[i] - origin[i]) / direction[i];
				else
					maxT[i] = -1.;

			/* Get largest of the maxT's for final choice of intersection */
			int whichPlane = 0;
			for (i = 1; i < NUMDIM; i++)
				if (maxT[whichPlane] < maxT[i])
					whichPlane = i;

			/* Check final candidate actually inside box */
			if (maxT[whichPlane] < 0.) return false;
			for (i = 0; i < NUMDIM; i++)
				if (whichPlane != i) {
					coord[i] = origin[i] + maxT[whichPlane] * direction[i];
					if (coord[i] < _min[i] || coord[i] > _max[i])
						return false;
				} else {
					coord[i] = candidatePlane[i];
				}
			return true;				/* ray hits box */
	}

}

bool AABB::aabbIntersects(const AABB& o) const {
	//TODO: Lo acaba de hacer el copilot. Si no funciona, no es mi culpa.
	return (_min.x <= o._max.x && _max.x >= o._min.x) &&
		(_min.y <= o._max.y && _max.y >= o._min.y) &&
		(_min.z <= o._max.z && _max.z >= o._min.z);
}

//std::ostream& operator<<(std::ostream& os, const AABB& aabb) {
//	os << "Maximum corner: " << aabb.max().x << ", " << aabb.max().y << ", " << aabb.max().z << "\n";
//	os << "Minimum corner: " << aabb.min().x << ", " << aabb.min().y << ", " << aabb.min().z << "\n";
//	return os;
//}

bool AABB::isInside(const vec3& point) const {
	return all(lessThanEqual((vec3)_min, point)) && all(lessThanEqual(point, (vec3)_max));
	//return (point.x >= _min.x) && (point.y >= _min.y) && (point.z >= _min.z) && (point.x <= _max.x) && (point.y <= _max.y) && (point.z <= _max.z);
}


