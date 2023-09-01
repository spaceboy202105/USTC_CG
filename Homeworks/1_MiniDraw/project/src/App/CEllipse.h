#pragma once

#include "CFigure.h"

class CEllipse : public CFigure {
public:
	CEllipse();
	~CEllipse();

	void Draw(QPainter& painter);
};