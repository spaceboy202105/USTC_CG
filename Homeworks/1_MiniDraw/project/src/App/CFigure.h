#pragma once

#include <QtGui>

class CFigure
{
public:
	CFigure();
	CFigure(QColor color);
	virtual ~CFigure();
	virtual void Draw(QPainter& paint) = 0;
	virtual void update(int mode) {}//for polygon
	void set_start(QPoint s);
	void set_end(QPoint e);
	void set_color(QColor c);
	QColor get_color();

public:
	enum Type
	{
		kDefault = 0,
		kLine = 1,
		kRect = 2,
		kEllipse = 3,
		KPolygon = 4,
		KFreehand = 5
	};

protected:
	QColor color;
	int width;
	QPoint start;
	QPoint end;
};
