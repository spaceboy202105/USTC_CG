#ifndef FREEDRAW_H
#define FREEDRAW_H

#include"CFigure.h"
#include <qpainterpath.h>

namespace poissonedit
{
	class Freedraw :public CFigure
	{
		public:
			Freedraw();
			~Freedraw();

			void Draw(QPainter& painter);
			QPainterPath get_path();

		protected:
			QPainterPath path;
	};
}


#endif