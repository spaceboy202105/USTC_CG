#pragma once

#include <QtGui>
#include "ImagePaste.h"

class CFigure
{
public:
	CFigure();
	virtual ~CFigure();
	void set_start(QPoint s);
	void set_end(QPoint e);
	QPoint get_start();
	QPoint get_end();
	virtual void Draw(QPainter& paint) = 0;
	virtual void update(int mode) {}
	virtual QPolygon get_points() { QPolygon polygon; return polygon; }
	virtual QPainterPath get_path() { QPainterPath path; return path; }

public:
	enum Type
	{
		kDefault,
		kRect,
		kEllipse,
		kPolygon,
		kFreedraw,
	};
	Type type_;

protected:
	QPoint start;
	QPoint end;
};