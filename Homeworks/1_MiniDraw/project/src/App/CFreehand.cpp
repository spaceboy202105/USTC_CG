#include "CFreehand.h"

CFreehand::CFreehand()
{
}

CFreehand::~CFreehand()
{
}

void CFreehand::Draw(QPainter& painter)
{
	/*the logic of freehand:
	 *1.when elementCount() == 0:pen up
	 *2.when mouse move a frame a line lock to
	 */
	if (path.elementCount() == 0)
		path.moveTo(end);
	else
		path.lineTo(end);
	painter.drawPath(path);
}