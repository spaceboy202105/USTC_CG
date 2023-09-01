#pragma once

#include"CFigure.h"
#include "ImagePaste.h"

namespace poissonedit
{
	class CRect :public CFigure
	{
	public:
		CRect();
		~CRect();

		void Draw(QPainter& painter);
	};
}
