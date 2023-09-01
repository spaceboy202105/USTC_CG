#include "CPolygon.h"

CPolygon::CPolygon()
{
	finish = false;
	polygonPath.push_back(start);
}

CPolygon::~CPolygon()
{
}

void CPolygon::Draw(QPainter& painter)
{
	//painter.drawPolygon();
	if (finish)painter.drawPolygon(polygonPath);//closed
	else painter.drawPolyline(polygonPath);//not closed
}

void CPolygon::update(int mode)
{
	if (mode == 0)
	{
		finish = true;
	}
	else if (mode == 1)
	{
		if (polygonPath.size() > 0)polygonPath.back() = end;
		polygonPath.push_back(polygonPath.back());
	}
	else
	{
		return;
	}
}