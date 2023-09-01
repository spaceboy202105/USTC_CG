#ifndef POLYGON_H
#define POLYGON_H

#include"CFigure.h"

namespace poissonedit {
	class CPolygon :public CFigure
	{
		public:
			CPolygon();
			~CPolygon();

			void Draw(QPainter& painter);
			void update(int mode);
			QPolygon get_points();

		private:
			QPolygon polygon;
			bool finish;
	};

}



#endif 