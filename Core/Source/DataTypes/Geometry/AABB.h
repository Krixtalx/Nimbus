#pragma once
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include <vector>
using namespace glm;

class AABB final {
protected:
	dvec3 _min, _max;
public:
	AABB(const dvec3& min = vec3(INFINITY), const dvec3& max = vec3(-INFINITY));
	AABB(const AABB& aabb);
	~AABB();
	AABB& operator=(const AABB& aabb);
	bool operator==(const AABB& aabb) const { return _min == aabb._min && _max == aabb._max; }
	bool operator!=(const AABB& aabb) const { return !(*this == aabb); }

	dvec3 center() const { return (_max + _min) / 2.0; }
	AABB dot(const mat4& matrix) const;
	vec3 extent() const { return _max - _min; }
	double extentLength() const { return distance(_min, _max); }
	vec3 max() const { return _max; }
	vec3 min() const { return _min; }
	vec3 size() const { return _max - _min; }
	std::vector<dvec3> corners() const;

	void update(const AABB& aabb);
	void update(const vec3& point);
	void update(const dvec3& point);

	//friend std::ostream& operator<<(std::ostream& os, const AABB& aabb);

	bool rayIntersects(const vec3& origin, const vec3& direction) const;
	bool aabbIntersects(const AABB& o) const;

	bool isInside(const vec3& point) const;
};

