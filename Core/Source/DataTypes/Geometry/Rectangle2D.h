#pragma once

/**
*	@file Rectangle2D.h
*	@authors Alfonso López Ruiz (alr00048@red.ujaen.es), José Antonio Collado Araque (jaca0011@red.ujaen.es)
*	@date 25/07/2024
*/

#define NUM_RECTANGLE_CORNERS 4

/**
*	@brief
*/
class Rectangle2D {
protected:
	enum RectangleCorner : uint8_t {
		BOTTOM_LEFT = 0, BOTTOM_RIGHT = 1, TOP_RIGHT = 2, TOP_LEFT = 3
	};

protected:
	vec2 _corners[4];			//!<

protected:
	/**
	*	@return True if triangle is defined counterclockwise direction.
	*/
	bool triangleIsCCW(const vec2& p1, const vec2& p2, const vec2& p3);

public:
	/**
	*	@brief Default constructor.
	*/
	Rectangle2D();


	/**
	*	@brief
	*/
	Rectangle2D(const vec2& min, const vec2& max);

	/**
	*	@brief
	*/
	Rectangle2D(const vec2& bottomLeft, const vec2& bottomRight, const vec2& topLeft, const vec2& topRight);

	/**
	*	@brief Copy constructor.
	*/
	Rectangle2D(const Rectangle2D& rectangle);

	/**
	*	@brief Destructor.
	*/
	virtual ~Rectangle2D();

	/**
	*	@brief Assignment operator overriding.
	*/
	Rectangle2D& operator=(const Rectangle2D& rectangle);

	/**
	*	@return
	*/
	bool exceedsSlope(float threshold_x, float threshold_y) const;

	/**
	*	@return True if point is inside our rectangle.
	*/
	bool isInside(const vec2& point) const;

	/**
	*	@return
	*/
	bool isInsideBinarySearch(const vec2& point);

	/**
	*	@brief
	*/
	Rectangle2D* removeSlope(const vec2& maxSize) const;

	/**
	*	@brief
	*/
	void scaleCorners(const vec2& scale);

	/**
	*	@brief
	*/
	void translateCorners(const vec2& translation);

	// Getters

	/**
	*	@return
	*/
	vec2 getBottomLeft() const { return _corners[0]; }

	/**
	*	@return
	*/
	vec2 getBottomRight() const { return _corners[1]; }

	/**
	*	@return
	*/
	vec2 getTopLeft() const { return _corners[3]; }

	/**
	*	@return
	*/
	vec2 getTopRight() const { return _corners[2]; }

	/**
	*	@return
	*/
	vec2 getSize() const;
};

