#include "CRect.h"

using namespace poissonedit;

CRect::CRect()
{
	type_ = kRect;
}

CRect::~CRect()
{
}

void CRect::Draw(QPainter& painter)
{
	painter.drawRect(start.x(), start.y(),
		end.x() - start.x(), end.y() - start.y());
}