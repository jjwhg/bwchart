#pragma once

/// <summary>
/// Bezier Spline methods
/// </summary>
class BezierSpline
{
public:
	/// <summary>
	/// Get open-ended Bezier Spline Control Points.
	/// </summary>
	/// <param name="knots">Input Knot Bezier spline points.</param>
	/// <param name="firstControlPoints">Output First Control points
	/// array of knots.Length - 1 length.</param>
	/// <param name="secondControlPoints">Output Second Control points
	/// array of knots.Length - 1 length.</param>
	/// <exception cref="ArgumentNullException"><paramref name="knots"/>
	/// parameter must be not null.</exception>
	/// <exception cref="ArgumentException"><paramref name="knots"/>
	/// array must contain at least two points.</exception>
	static bool GetCurveControlPoints(CPoint* knots, int count, CPoint*& firstControlPoints, CPoint*& secondControlPoints);

private:
	/// <summary>
	/// Solves a tridiagonal system for one of coordinates (x or y)
	/// of first Bezier control points.
	/// </summary>
	/// <param name="rhs">Right hand side vector.</param>
	/// <returns>Solution vector.</returns>
	static double* GetFirstControlPoints(double* rhs, int count);
};
