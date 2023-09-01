#pragma once

#include "CFigure.h"

class Rect : public CFigure {
public:
	Rect();
	~Rect();

	void Draw(QPainter& painter);
};

