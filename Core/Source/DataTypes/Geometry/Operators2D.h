#pragma once

namespace Nimbus::Operators2D {
	/**
	 * \brief Projects vector a over b
	 * \param a Vector target of projection
	 * \param b Vector to project a
	 * \return Projected vector
	 */
	vec2 projectVector(vec2 a, vec2 b);

	/**
	 * \brief Projects point into given vector
	 * \param p Point to proyect
	 * \param v Vector to proyect to
	 * \return Projection t value (from line equation p = v1 + t (v2 - v1))
	 */
	float projectPoint01(vec2 p, vec2 v);

	/**
	 * \brief Checks if given point is inside given rectangle
	 * \param point Point to test
	 * \param rectA Rectangle corner
	 * \param rectB Opposite corner to rectA
	 * \return true if point is inside rectangle or in edge; false otherwise
	 */
	bool checkPointInRect(vec2 point, vec2 rectA, vec2 rectB);

	/**
	 * \brief Checks if two given rectangles collide
	 * \param rectA1 First rectangle corner
	 * \param rectA2 First rectangle opposite corner
	 * \param rectB1 Second rectangle corner
	 * \param rectB2 Second rectangle opposite corner
	 * \return
	 */
	bool checkRectToRect(vec2 rectA1, vec2 rectA2, vec2 rectB1, vec2 rectB2);

	bool checkAABBoxToAABBox(vec2 boxAmin, vec2 boxAmax, vec2 boxBmin, vec2 boxBmax);
	bool checkAABBToAABBAnyAxisIntersect(vec2 boxAmin, vec2 boxAmax, vec2 boxBmin, vec2 boxBmax);

	/**
	 * @brief Checks if boxA is inside boxB
	 * @param boxAmin
	 * @param boxAmax
	 * @param boxBmin
	 * @param boxBmax
	 * @return
	 */
	bool checkAABBinAABB(vec2 boxAmin, vec2 boxAmax, vec2 boxBmin, vec2 boxBmax);
}

inline vec2 Nimbus::Operators2D::projectVector(const vec2 a, const vec2 b) {
	const auto k = dot(a, b) / dot(b, b);
	return b * k;
}

inline float Nimbus::Operators2D::projectPoint01(const vec2 p, const vec2 v) {
	const float vDist = distance(v.x, v.y);
	const vec2 vp = p - v;

	return (dot(vp, v)) / vDist;
}


inline bool Nimbus::Operators2D::checkPointInRect(const vec2 point, const vec2 rectA, const vec2 rectB) {
	const auto minRectX = std::min(rectA.x, rectB.x);
	const auto maxRectX = std::max(rectA.x, rectB.x);
	const auto minRectY = std::min(rectA.y, rectB.y);
	const auto maxRectY = std::max(rectA.y, rectB.y);

	return
		point.x <= maxRectX && point.x >= minRectX &&
		point.y <= maxRectY && point.y >= minRectY;
}

inline bool Nimbus::Operators2D::checkRectToRect(vec2 rectA1, vec2 rectA2, vec2 rectB1, vec2 rectB2) {
	// Get each rect vertices and edges
	const vec2 rectAExtent = (rectA2 - rectA1) * 0.5f;
	const float extentAModule = length(rectAExtent);
	const vec2 rectACenter = rectA1 + rectAExtent;
	const vec2 rectBExtent = (rectB2 - rectB1) * 0.5f;
	const float extentBModule = length(rectBExtent);
	const vec2 rectBCenter = rectB1 + rectBExtent;

	const vec2 a0 = { rectACenter.x - rectAExtent.x, rectACenter.y - rectAExtent.y };
	const vec2 a1 = { rectACenter.x - rectAExtent.x, rectACenter.y + rectAExtent.y };
	const vec2 a2 = { rectACenter.x + rectAExtent.x, rectACenter.y + rectAExtent.y };
	const vec2 a3 = { rectACenter.x + rectAExtent.x, rectACenter.y - rectAExtent.y };
	const vec2 b0 = { rectBCenter.x - rectBExtent.x, rectBCenter.y - rectBExtent.y };
	const vec2 b1 = { rectBCenter.x - rectBExtent.x, rectBCenter.y + rectBExtent.y };
	const vec2 b2 = { rectBCenter.x + rectBExtent.x, rectBCenter.y + rectBExtent.y };
	const vec2 b3 = { rectBCenter.x + rectBExtent.x, rectBCenter.y - rectBExtent.y };

	//https://www.codezealot.org/archives/55/
	std::array rectsPointsA = {
		a1 - a0,
		a2 - a1,
		a3 - a2,
		a0 - a3
	};
	std::array rectsPointsB = {
		b1 - b0,
		b2 - b1,
		b3 - b2,
		b0 - b3
	};
	std::vector<vec2> testAxes = {
		{-rectsPointsA[0].y, rectsPointsA[0].x},
		{-rectsPointsA[1].y, rectsPointsA[1].x},
		{-rectsPointsA[2].y, rectsPointsA[2].x},
		{-rectsPointsA[3].y, rectsPointsA[3].x},
		{-rectsPointsB[0].y, rectsPointsB[0].x},
		{-rectsPointsB[1].y, rectsPointsB[1].x},
		{-rectsPointsB[2].y, rectsPointsB[2].x},
		{-rectsPointsB[3].y, rectsPointsB[3].x}
	};

	rectsPointsA = {
		a0,a1,a2,a3
	};
	rectsPointsB = {
		b0,b1,b2,b3
	};

	// Precheck by distance
	{
		int insideCount = 0;
		for (auto& point : rectsPointsA) {
			float dist = distance(point, rectBCenter);
			insideCount += dist <= extentBModule ? 1 : 0;
		}
		for (auto& point : rectsPointsB) {
			float dist = distance(point, rectACenter);
			insideCount += dist <= extentAModule ? 1 : 0;
		}
		if (insideCount == 0) {
			return false;
		}
	}

	//Check all axes projections
	for (auto& axis : testAxes) {
		// Get proyection range for both rects
		float maxT_A = -FLT_MAX, minT_A = FLT_MAX;
		for (auto& point : rectsPointsA) {
			float t = projectPoint01(point, axis);
			maxT_A = std::max(maxT_A, t);
			minT_A = std::min(minT_A, t);
		}

		float maxT_B = -FLT_MAX, minT_B = FLT_MAX;
		for (auto& point : rectsPointsB) {
			float t = projectPoint01(point, axis);
			maxT_B = std::max(maxT_B, t);
			minT_B = std::min(minT_B, t);
		}

		//Check if proyection intersects. When any axis returns no intersection, rects doesn't intersect
		bool projIntersects =
			(minT_A <= maxT_B || minT_A >= minT_B) ||
			(maxT_A <= maxT_B || maxT_A >= minT_B) ||
			(minT_B <= maxT_A || minT_B >= minT_A) ||
			(maxT_B <= maxT_A || maxT_B >= minT_A);

		if (!projIntersects) {
			return false;
		}
	}
	return true;
}

inline bool Nimbus::Operators2D::checkAABBoxToAABBox(const vec2 boxAmin, const vec2 boxAmax, const vec2 boxBmin, const vec2 boxBmax) { //FIXME: no detecta A totalmente contenida en B
	const bool xIntersect =
		(boxAmin.x >= boxBmin.x && boxAmin.x <= boxBmax.x) || (boxAmax.x >= boxBmin.x && boxAmax.x <= boxBmax.x) ||
		(boxBmin.x >= boxAmin.x && boxBmin.x <= boxAmax.x) || (boxBmax.x >= boxAmin.x && boxBmax.x <= boxAmax.x);
	const bool yIntersect =
		(boxAmin.y >= boxBmin.y && boxAmin.y <= boxBmax.y) || (boxAmax.y >= boxBmin.y && boxAmax.y <= boxBmax.y) ||
		(boxBmin.y >= boxAmin.y && boxBmin.y <= boxAmax.y) || (boxBmax.y >= boxAmin.y && boxBmax.y <= boxAmax.y);

	return xIntersect && yIntersect;

	// vec2 boxAExtent = (boxAmax - boxAmin);
	// vec2 boxBExtent = (boxBmax - boxBmin);
	//
	// vec2 boxACenter = boxAmin + boxAExtent * 0.5f;
	// vec2 boxBCenter = boxBmin + boxBExtent * 0.5f;
	//
	// return
	// 	(abs(boxACenter.x - boxBCenter.x) * 2 < (boxAExtent.x + boxBExtent.x)) &&
	// 	(abs(boxACenter.y - boxBCenter.y) * 2 < (boxAExtent.y + boxBExtent.y));
}

inline bool Nimbus::Operators2D::checkAABBToAABBAnyAxisIntersect(const vec2 boxAmin, const vec2 boxAmax, const vec2 boxBmin, const vec2 boxBmax) {
	const bool xIntersect =
		(boxAmin.x >= boxBmin.x && boxAmin.x <= boxBmax.x) || (boxAmax.x >= boxBmin.x && boxAmax.x <= boxBmax.x) ||
		(boxBmin.x >= boxAmin.x && boxBmin.x <= boxAmax.x) || (boxBmax.x >= boxAmin.x && boxBmax.x <= boxAmax.x);
	const bool yIntersect =
		(boxAmin.y >= boxBmin.y && boxAmin.y <= boxBmax.y) || (boxAmax.y >= boxBmin.y && boxAmax.y <= boxBmax.y) ||
		(boxBmin.y >= boxAmin.y && boxBmin.y <= boxAmax.y) || (boxBmax.y >= boxAmin.y && boxBmax.y <= boxAmax.y);

	return xIntersect || yIntersect;
}

inline bool Nimbus::Operators2D::checkAABBinAABB(const vec2 boxAmin, const vec2 boxAmax, const vec2 boxBmin, const vec2 boxBmax) {
	return (boxAmin.x >= boxBmin.x && boxAmin.y >= boxBmin.y && boxAmax.x <= boxBmax.x && boxAmax.y <= boxBmax.y);
}

