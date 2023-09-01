#pragma once

#include "CFigure.h"

class Line : public CFigure {
public:
	Line();
	~Line();

	void Draw(QPainter& painter);
};
