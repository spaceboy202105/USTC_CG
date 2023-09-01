#pragma once

#include <ui_viewwidget.h>

#include "CFigure.h"
#include "Line.h"
#include "Rect.h"
#include "CEllipse.h"
#include "CPolygon.h"
#include <qevent.h>
#include <qpainter.h>
#include <QWidget>
#include <QColorDialog>
#include <vector>

class ViewWidget : public QWidget
{
	Q_OBJECT

public:
	ViewWidget(QWidget* parent = 0);
	~ViewWidget();

private:
	Ui::ViewWidget ui;

private:
	bool draw_status_;
	QPoint start_point_;
	QPoint end_point_;
	QColor curren_tcolor;
	CFigure::Type type_;
	CFigure* shape_;//Now Shape
	std::vector<CFigure*> shape_list_;

public:
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

public:
	void paintEvent(QPaintEvent*);
signals:
public slots:
	void setLine();
	void setRect();
	void setEllipse();
	void setFreehand();
	void setPolygon();
	void setColor();
	void undoPaint();
};
