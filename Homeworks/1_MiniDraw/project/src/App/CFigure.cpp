#include "CFigure.h"

CFigure::CFigure()
{
	color = QColor(255, 255, 255);
	width = 1;
}

CFigure::CFigure(QColor color)
{
	this->color = color;
	width = 1;
}

CFigure::~CFigure()
{
}

void CFigure::set_start(QPoint s)
{
	start = s;
}

void CFigure::set_end(QPoint e)
{
	end = e;
}

void CFigure::set_color(QColor c)
{
	this->color = c;
}

QColor CFigure::get_color()
{
	return color;
}