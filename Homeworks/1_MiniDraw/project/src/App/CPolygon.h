#include "CFigure.h"
class CPolygon :public CFigure
{
public:
	CPolygon();
	~CPolygon();
	void Draw(QPainter& painter);
	void update(int mode);

protected:

	QPolygon polygonPath;//the last shape
	bool finish;
};