#pragma once

#include "CFigure.h"

class CFreehand : public CFigure {
public:
	CFreehand();
	~CFreehand();

	void Draw(QPainter& painter);
private:
	QPainterPath path;//the path which the mouse move
};