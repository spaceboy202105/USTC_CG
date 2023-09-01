#include "CEllipse.h"

CEllipse::CEllipse()
{
	//Type = kEllipse;
}

CEllipse::~CEllipse()
{
}

void CEllipse::Draw(QPainter& painter)
{
	painter.drawEllipse(start.x(), start.y(), end.x() - start.x(), end.y() - start.y());
}