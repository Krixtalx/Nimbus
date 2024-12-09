#pragma once

namespace Nimbus::Operators3D {
	/**
	 * \brief Check intersection between AABB and ray. Adapted from https://github.com/erich666/GraphicsGems/blob/master/gems/RayBox.c
	 * \param min AABB min corner
	 * \param max AABB max corner
	 * \param rayOrig Ray rayOrig point
	 * \param rayDir Ray direction vector
	 * \param hitPos Intersection position, if any
	 * \return true if ray intersects AABB, false otherwise
	 */
	bool checkRayAABB(vec3 min, vec3 max, vec3 rayOrig, vec3 rayDir, vec3& hitPos);

	double pointToLineDistance(dvec3 point, dvec3 lineA, dvec3 lineB);
}

inline bool Nimbus::Operators3D::checkRayAABB(vec3 min, vec3 max, vec3 rayOrig, vec3 rayDir, vec3& hitPos) {
	enum {
		NUMDIM = 3,
		RIGHT = 0,
		LEFT = 1,
		MIDDLE = 2
	};

	bool inside = true;
	char quadrant[3];
	int i;
	double maxT[3];
	double candidatePlane[3];

	/* Find candidate planes; this loop can be avoided if
	rays cast all from the eye(assume perpsective view) */
	for (i = 0; i < 3; i++)
		if (rayOrig[i] < min[i]) {
			quadrant[i] = LEFT;
			candidatePlane[i] = min[i];
			inside = false;
		} else if (rayOrig[i] > max[i]) {
			quadrant[i] = RIGHT;
			candidatePlane[i] = max[i];
			inside = false;
		} else {
			quadrant[i] = MIDDLE;
		}

		/* Ray rayOrig inside bounding box */
		if (inside) {
			hitPos = rayOrig;
			return true;
		}


		/* Calculate T distances to candidate planes */
		for (i = 0; i < 3; i++)
			if (quadrant[i] != MIDDLE && rayDir[i] != 0.)
				maxT[i] = (candidatePlane[i] - rayOrig[i]) / rayDir[i];
			else
				maxT[i] = -1.;

		/* Get largest of the maxT's for final choice of intersection */
		int whichPlane = 0;
		for (i = 1; i < 3; i++)
			if (maxT[whichPlane] < maxT[i])
				whichPlane = i;

		/* Check final candidate actually inside box */
		if (maxT[whichPlane] < 0.) return false;
		for (i = 0; i < 3; i++)
			if (whichPlane != i) {
				hitPos[i] = rayOrig[i] + maxT[whichPlane] * rayDir[i];
				if (hitPos[i] < min[i] || hitPos[i] > max[i])
					return false;
			} else {
				hitPos[i] = candidatePlane[i];
			}
		return true;				/* ray hits box */
}

inline double Nimbus::Operators3D::pointToLineDistance(dvec3 point, dvec3 lineA, dvec3 lineB) {
	dvec3 d = (lineB - lineA) / distance(lineB, lineA);
	dvec3 v = point - lineA;
	double t = dot(v, d);
	dvec3 linePoint = lineA + d * t;
	return distance(linePoint, point);

	/*
	double computeDistance(vec3 A, vec3 B, vec3 C) {
		vec3 d = (C - B) / C.distance(B);
		vec3 v = A - B;
		double t = v.dot(d);
		vec3 P = B + t * d;
		return P.distance(A);
	}
	*/
}
