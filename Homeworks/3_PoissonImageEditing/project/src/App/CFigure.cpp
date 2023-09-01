#include "CFigure.h"

CFigure::CFigure()
{
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

QPoint CFigure::get_start()
{
	return start;
}

QPoint CFigure::get_end()
{
	return end;
}