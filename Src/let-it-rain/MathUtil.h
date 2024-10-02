#pragma once
#include "windef.h"
#include <d2d1.h>
#include <algorithm>

#define RECT_WIDTH(rect)  ((rect).right - (rect).left)
#define RECT_HEIGHT(rect) ((rect).bottom - (rect).top)

class MathUtil
{
public:
	// Function to check if a point is inside or on the edge of a RECT
	static bool IsPointInRect(const RECT& rect, const D2D1_POINT_2F& point)
	{
		// Check if the point is within or on the boundary of the rectangle
		return (point.x >= rect.left && point.x <= rect.right && point.y >= rect.top && point.y <= rect.bottom);
	}

	// Helper function to calculate intersection
	static bool LineIntersect(const D2D1_POINT_2F& p1, const D2D1_POINT_2F& p2, const D2D1_POINT_2F& q1,
	                          const D2D1_POINT_2F& q2, D2D1_POINT_2F& intersection)
	{
		float A1 = p2.y - p1.y;
		float B1 = p1.x - p2.x;
		float C1 = A1 * p1.x + B1 * p1.y;

		float A2 = q2.y - q1.y;
		float B2 = q1.x - q2.x;
		float C2 = A2 * q1.x + B2 * q1.y;

		float det = A1 * B2 - A2 * B1;
		if (det == 0) return false; // Parallel lines

		intersection.x = (B2 * C1 - B1 * C2) / det;
		intersection.y = (A1 * C2 - A2 * C1) / det;

		return (intersection.x >= (std::min)(p1.x, p2.x) && intersection.x <= (std::max)(p1.x, p2.x) &&
			intersection.y >= (std::min)(p1.y, p2.y) && intersection.y <= (std::max)(p1.y, p2.y));
	}

	static void TrimLineSegment(const RECT& boundRect, const D2D1_POINT_2F& lineStart, const D2D1_POINT_2F& lineEnd,
	                            D2D1_POINT_2F& lineTrimmedStart, D2D1_POINT_2F& lineTrimmedEnd)
	{
		D2D1_POINT_2F rectPoints[4] = {
			{static_cast<float>(boundRect.left), static_cast<float>(boundRect.top)},
			{static_cast<float>(boundRect.right), static_cast<float>(boundRect.top)},
			{static_cast<float>(boundRect.right), static_cast<float>(boundRect.bottom)},
			{static_cast<float>(boundRect.left), static_cast<float>(boundRect.bottom)}
		};

		lineTrimmedStart = lineStart;
		lineTrimmedEnd = lineEnd;

		for (int i = 0; i < 4; ++i)
		{
			int next = (i + 1) % 4;
			D2D1_POINT_2F intersection;
			if (LineIntersect(lineStart, lineEnd, rectPoints[i], rectPoints[next], intersection))
			{
				if (!(lineTrimmedStart.x >= boundRect.left && lineTrimmedStart.x <= boundRect.right &&
					lineTrimmedStart.y >= boundRect.top && lineTrimmedStart.y <= boundRect.bottom))
				{
					lineTrimmedStart = intersection;
				}
				else
				{
					lineTrimmedEnd = intersection;
				}
			}
		}

		// Clamp to rect bounds
		lineTrimmedStart.x = std::clamp(lineTrimmedStart.x, static_cast<float>(boundRect.left),
		                                static_cast<float>(boundRect.right));
		lineTrimmedStart.y = std::clamp(lineTrimmedStart.y, static_cast<float>(boundRect.top),
		                                static_cast<float>(boundRect.bottom));
		lineTrimmedEnd.x = std::clamp(lineTrimmedEnd.x, static_cast<float>(boundRect.left),
		                              static_cast<float>(boundRect.right));
		lineTrimmedEnd.y = std::clamp(lineTrimmedEnd.y, static_cast<float>(boundRect.top),
		                              static_cast<float>(boundRect.bottom));
	}

	static RECT SubtractRect(const RECT& monitorRect, const RECT& taskBarRect)
	{
		RECT result = {0, 0, 0, 0};

		// Check if the taskBarRect is on the top edge and spans the full width
		if (taskBarRect.top == monitorRect.top && taskBarRect.right == monitorRect.right && taskBarRect.left ==
			monitorRect.left)
		{
			// Bottom remaining rectangle
			result = {monitorRect.left, taskBarRect.bottom, monitorRect.right, monitorRect.bottom};
		}
		// Check if the taskBarRect is on the bottom edge and spans the full width
		else if (taskBarRect.bottom == monitorRect.bottom && taskBarRect.right == monitorRect.right && taskBarRect.left
			== monitorRect.left)
		{
			// Top remaining rectangle
			result = {monitorRect.left, monitorRect.top, monitorRect.right, taskBarRect.top};
		}
		// Check if the taskBarRect is on the left edge and spans the full height
		else if (taskBarRect.left == monitorRect.left && taskBarRect.top == monitorRect.top && taskBarRect.bottom ==
			monitorRect.bottom)
		{
			// Right remaining rectangle
			result = {taskBarRect.right, monitorRect.top, monitorRect.right, monitorRect.bottom};
		}
		// Check if the taskBarRect is on the right edge and spans the full height
		else if (taskBarRect.right == monitorRect.right && taskBarRect.top == monitorRect.top && taskBarRect.bottom ==
			monitorRect.bottom)
		{
			// Left remaining rectangle
			result = {monitorRect.left, monitorRect.top, taskBarRect.left, monitorRect.bottom};
		}
		return result;
	}
};