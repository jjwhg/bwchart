#include "stdafx.h"
#include "bezier.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
bool BezierSpline::GetCurveControlPoints(CPoint* knots, int count, CPoint*& firstControlPoints, CPoint*& secondControlPoints)
{
	if (knots == 0)
		return false;

	int n = count - 1;
	if (n < 1)
		return false;

	if (n == 1)
	{ // Special case: Bezier curve should be a straight line.
		firstControlPoints = new CPoint[1];
		// 3P1 = 2P0 + P3
		firstControlPoints[0].x = (2 * knots[0].x + knots[1].x) / 3;
		firstControlPoints[0].y = (2 * knots[0].y + knots[1].y) / 3;

		secondControlPoints = new CPoint[1];
		// P2 = 2P1 – P0
		secondControlPoints[0].x = 2 *
			firstControlPoints[0].x - knots[0].x;
		secondControlPoints[0].y = 2 *
			firstControlPoints[0].y - knots[0].y;
		return true;
	}

	// Calculate first Bezier control points
	// Right hand side vector
	double* rhs = new double[n];

	// Set right hand side X values
	for (int i = 1; i < n - 1; ++i)
		rhs[i] = 4 * knots[i].x + 2 * knots[i + 1].x;
	rhs[0] = knots[0].x + 2 * knots[1].x;
	rhs[n - 1] = (8 * knots[n - 1].x + knots[n].x) / 2.0;
	// Get first control points X-values
	double* x = GetFirstControlPoints(rhs,n);

	// Set right hand side Y values
	for (int i = 1; i < n - 1; ++i)
		rhs[i] = 4 * knots[i].y + 2 * knots[i + 1].y;
	rhs[0] = knots[0].y + 2 * knots[1].y;
	rhs[n - 1] = (8 * knots[n - 1].y + knots[n].y) / 2.0;
	// Get first control points Y-values
	double* y = GetFirstControlPoints(rhs,n);

	// Fill output arrays.
	firstControlPoints = new CPoint[n];
	secondControlPoints = new CPoint[n];
	for (int i = 0; i < n; ++i)
	{
		// First control point
		firstControlPoints[i] = CPoint((int)x[i], (int)y[i]);
		// Second control point
		if (i < n - 1)
			secondControlPoints[i] = CPoint((int)(2 * knots[i + 1].x - x[i + 1]), (int)(2 *	knots[i + 1].y - y[i + 1]));
		else
			secondControlPoints[i] = CPoint((int)((knots[n].x + x[n - 1]) / 2), (int)((knots[n].y + y[n - 1]) / 2));
	}

	delete[]rhs;
	delete[]x;
	delete[]y;
	return true;
}

/// <summary>
/// Solves a tridiagonal system for one of coordinates (x or y)
/// of first Bezier control points.
/// </summary>
/// <param name="rhs">Right hand side vector.</param>
/// <returns>Solution vector.</returns>
double* BezierSpline::GetFirstControlPoints(double* rhs, int count)
{
	int n = count;
	double* x = new double[n]; // Solution vector.
	double* tmp = new double[n]; // Temp workspace.

	double b = 2.0;
	x[0] = rhs[0] / b;
	for (int i = 1; i < n; i++) // Decomposition and forward substitution.
	{
		tmp[i] = 1 / b;
		b = (i < n - 1 ? 4.0 : 3.5) - tmp[i];
		x[i] = (rhs[i] - x[i - 1]) / b;
	}
	for (int i = 1; i < n; i++)
		x[n - i - 1] -= tmp[n - i] * x[n - i]; // Backsubstitution.

	delete[]tmp;

	return x;
}
