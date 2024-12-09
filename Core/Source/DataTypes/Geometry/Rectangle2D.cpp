#include "CorePch.h"
#include "Rectangle2D.h"

/// Public methods

Rectangle2D::Rectangle2D() {
	for (auto& _corner : _corners) {
		_corner = vec2(.0f);
	}
}

Rectangle2D::Rectangle2D(const vec2& min, const vec2& max)
{
	_corners[BOTTOM_LEFT] = min;
	_corners[BOTTOM_RIGHT] = {max.x, min.y};
	_corners[TOP_LEFT] = { min.x, max.y };
	_corners[TOP_RIGHT] = max;
}

Rectangle2D::Rectangle2D(const vec2& bottomLeft, const vec2& bottomRight, const vec2& topLeft, const vec2& topRight) {
	_corners[BOTTOM_LEFT] = bottomLeft;
	_corners[BOTTOM_RIGHT] = bottomRight;
	_corners[TOP_RIGHT] = topRight;
	_corners[TOP_LEFT] = topLeft;
}

Rectangle2D::Rectangle2D(const Rectangle2D& rectangle) {
	for (int image = 0; image < NUM_RECTANGLE_CORNERS; ++image) {
		this->_corners[image] = rectangle._corners[image];
	}
}

Rectangle2D::~Rectangle2D() = default;

Rectangle2D& Rectangle2D::operator=(const Rectangle2D& rectangle) {
	for (int image = 0; image < NUM_RECTANGLE_CORNERS; ++image) {
		this->_corners[image] = rectangle._corners[image];
	}

	return *this;
}

bool Rectangle2D::exceedsSlope(const float threshold_x, const float threshold_y) const {
	vec2 diff_x = vec2(glm::abs(_corners[BOTTOM_LEFT].x - _corners[TOP_LEFT].x), glm::abs(_corners[BOTTOM_RIGHT].x - _corners[TOP_RIGHT].x)) - threshold_x;
	vec2 diff_y = vec2(glm::abs(_corners[BOTTOM_LEFT].y - _corners[BOTTOM_RIGHT].y), glm::abs(_corners[TOP_LEFT].y - _corners[TOP_RIGHT].y)) - threshold_y;

	return diff_x.x >= 0 || diff_x.y >= 0 || diff_y.x >= 0 || diff_y.y >= 0;
}

bool Rectangle2D::isInside(const vec2& point) const {
	return (point.x >= _corners[BOTTOM_LEFT].x) && (point.x <= _corners[BOTTOM_RIGHT].x) && (point.y >= _corners[BOTTOM_LEFT].y) && (point.y <= _corners[TOP_LEFT].y);
}

bool Rectangle2D::isInsideBinarySearch(const vec2& point) {
	// Do binary search over polygon vertices to find the fan triangle (v[0], v[low], v[high])
	int low = 0, high = NUM_RECTANGLE_CORNERS;

	do {
		int mid = (low + high) / 2;

		if (triangleIsCCW(_corners[0], _corners[mid], point)) {
			low = mid;
		} else {
			high = mid;
		}

	} while (low + 1 < high);

	// If point outside last (or first) edge, then it is not inside the n-gon
	if (low == 0 || high == NUM_RECTANGLE_CORNERS) return 0;

	// point is inside the polygon if it is left of the directed edge from v[low] to v[high]
	return triangleIsCCW(_corners[low], _corners[high], point);
}

Rectangle2D* Rectangle2D::removeSlope(const vec2& maxSize) const {
	float min_x = clamp(ceil(max(_corners[BOTTOM_LEFT].x, _corners[TOP_LEFT].x)), .0f, maxSize.x);
	float max_x = clamp(floor(min(_corners[BOTTOM_RIGHT].x, _corners[TOP_RIGHT].x)), .0f, maxSize.x);
	float min_y = clamp(ceil(max(_corners[BOTTOM_LEFT].y, _corners[BOTTOM_RIGHT].y)), .0f, maxSize.y);
	float max_y = clamp(floor(min(_corners[TOP_LEFT].y, _corners[TOP_RIGHT].y)), .0f, maxSize.y);

	return new Rectangle2D(vec2(min_x, min_y), vec2(max_x, min_y), vec2(min_x, max_y), vec2(max_x, max_y));
}

void Rectangle2D::scaleCorners(const vec2& scale) {
	for (auto& _corner : _corners) {
		_corner = scale * _corner;
	}
}

void Rectangle2D::translateCorners(const vec2& translation) {
	for (auto& _corner : _corners) {
		_corner = translation + _corner;
	}
}

vec2 Rectangle2D::getSize() const {
	const float min_x = std::min(_corners[BOTTOM_LEFT].x, _corners[TOP_LEFT].x);
	const float min_y = std::min(_corners[BOTTOM_LEFT].y, _corners[BOTTOM_RIGHT].y);
	const float max_x = std::max(_corners[BOTTOM_RIGHT].x, _corners[TOP_RIGHT].x);
	const float max_y = std::max(_corners[TOP_RIGHT].y, _corners[TOP_LEFT].y);

	return vec2(max_x - min_x, max_y - min_y);
}

/// Protected methods

bool Rectangle2D::triangleIsCCW(const vec2& p1, const vec2& p2, const vec2& p3) {
	const vec2 u = p2 - p1, v = p3 - p1;

	return (u.x * v.y - u.y * v.x) >= 0;
}