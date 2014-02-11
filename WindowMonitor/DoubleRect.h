#pragma once

class DoubleRect
{
public:
	double left, top, right, bottom;

	DoubleRect() :
		left(0.0), 
		top(0.0), 
		right(0.0), 
		bottom(0.0)
	{
	}

	void CopyFrom(DoubleRect const & other)
	{
		bottom = other.bottom;
		left = other.left;
		right = other.right;
		top = other.top;
	}
};