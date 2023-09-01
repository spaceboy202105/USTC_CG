#include "viewwidget.h"

#include "CFreehand.h"

ViewWidget::ViewWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	draw_status_ = false;
	shape_ = NULL;
	type_ = CFigure::kDefault;
}

ViewWidget::~ViewWidget()
{
}

void ViewWidget::setLine()
{
	type_ = CFigure::kLine;
}

void ViewWidget::setRect()
{
	type_ = CFigure::kRect;
}

void ViewWidget::setEllipse()
{
	type_ = CFigure::kEllipse;
}

void ViewWidget::setFreehand()
{
	type_ = CFigure::KFreehand;
}

void ViewWidget::setPolygon()
{
	type_ = CFigure::KPolygon;
}

void ViewWidget::setColor()
{
	QColor color = QColorDialog::getColor(Qt::red, this,
		tr("ÑÕÉ«¶Ô»°¿ò£¡"),
		QColorDialog::ShowAlphaChannel);
	this->curren_tcolor = color;
}

void ViewWidget::undoPaint()
{
	if (shape_list_.size()!=0)shape_list_.pop_back();
}

void ViewWidget::mousePressEvent(QMouseEvent* event)
{
	/*
	 * When mouse press:
	 * 1.check status
	 * 2.new obj by status code
	 * 3.set draw_status_ true
	 * 4.set start and end position
	 */
	if (Qt::LeftButton == event->button())
	{
		switch (type_)
		{
		case CFigure::kLine:
			shape_ = new Line();
			break;
		case CFigure::kDefault:
			break;
		case CFigure::kRect:
			shape_ = new Rect();
			break;
		case CFigure::kEllipse:
			shape_ = new CEllipse();
			break;
		case CFigure::KPolygon:
			if (shape_ == NULL)
			{
				shape_ = new CPolygon();
				setMouseTracking(true);
			}
			break;
		case CFigure::KFreehand:
			shape_ = new CFreehand();
			break;
		}
		if (shape_ != NULL)
		{
			draw_status_ = true;
			start_point_ = end_point_ = event->pos();
			shape_->set_color(curren_tcolor);
			shape_->set_start(start_point_);
			shape_->set_end(end_point_);
		}
	}
	update();
}

void ViewWidget::mouseMoveEvent(QMouseEvent* event)//Runtime Event
{
	if (draw_status_ && shape_ != NULL)
	{
		end_point_ = event->pos();
		shape_->set_end(end_point_);
	}
}

void ViewWidget::mouseReleaseEvent(QMouseEvent* event)
{	/* when mouse release :
	 *1.end draw
	 *2.store obj
	 *3.now shape_ptr to NULL
	 */
	if (shape_ != NULL)
	{
		if (type_ == CFigure::KPolygon)
		{
			if (Qt::LeftButton == event->button())
			{
				shape_->update(1);
			}
			else if (Qt::RightButton == event->button())
			{
				shape_->update(0);
				shape_list_.push_back(shape_);
				shape_ = NULL;
			}
		}
		else
		{
			draw_status_ = false;
			shape_list_.push_back(shape_);
			shape_ = NULL;
		}
	}
}

void ViewWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	for (int i = 0; i < shape_list_.size(); i++)//foreach frame paint all obj
	{
		painter.setPen(shape_list_[i]->get_color());
		shape_list_[i]->Draw(painter);
	}

	if (shape_ != NULL) {//paint current shape
		painter.setPen(shape_->get_color());
		shape_->Draw(painter);
	}

	update();
}