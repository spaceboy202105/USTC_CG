#include "CPolygon.h"

using namespace poissonedit;

CPolygon::CPolygon()
{
	type_ = kPolygon;
	finish = false;
	polygon.push_back(start);
}

CPolygon::~CPolygon()
{
}

QPolygon CPolygon::get_points()
{
	return polygon;
}

void CPolygon::update(int mode)
{
	switch (mode)
	{
	case 0:
		finish = true;
		break;
	case 1:
		if (polygon.size() > 0)
			polygon.back() = end;
		polygon.push_back(polygon.back());
		break;
	default:
		break;
	}
}

void CPolygon::Draw(QPainter& painter)
{
	if (finish)
		painter.drawPolygon(polygon);
	else
		painter.drawPolyline(polygon);
}