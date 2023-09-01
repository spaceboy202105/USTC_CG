#include "minidraw.h"

#include <QToolBar>

MiniDraw::MiniDraw(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	view_widget_ = new ViewWidget();
	Creat_Action();
	Creat_ToolBar();
	Creat_Menu();

	setCentralWidget(view_widget_);
}

void MiniDraw::Creat_Action() {
	Action_About = new QAction(tr("&About"), this);
	connect(Action_About, &QAction::triggered, this, &MiniDraw::AboutBox);

	Action_Line = new QAction(tr("&Line"), this);
	connect(Action_Line, SIGNAL(triggered()), view_widget_, SLOT(setLine()));

	Action_Rect = new QAction(tr("&Rect"), this);
	connect(Action_Rect, &QAction::triggered, view_widget_, &ViewWidget::setRect);

	Action_Ellipse = new QAction(tr("&Ellipse"), this);
	connect(Action_Ellipse, &QAction::triggered, view_widget_, &ViewWidget::setEllipse);

	Action_Polygon = new QAction(tr("&Polygon"), this);
	connect(Action_Polygon, &QAction::triggered, view_widget_, &ViewWidget::setPolygon);

	Action_Freehand = new QAction(tr("&Freehand"), this);
	connect(Action_Freehand, &QAction::triggered, view_widget_, &ViewWidget::setFreehand);
	/*Action_Freehand = new QAction(tr("&Freehand"), this);
	connect(Action_Freehand, &QAction::triggered, view_widget_, &ViewWidget::setFreehand);*/

	Action_Color = new QAction(tr("&Color"), this);
	connect(Action_Color, &QAction::triggered, view_widget_, &ViewWidget::setColor);

	Action_Undo = new QAction(tr("&Undo"), this);
	connect(Action_Undo, &QAction::triggered, view_widget_, &ViewWidget::undoPaint);
}

void MiniDraw::Creat_ToolBar() {
	pToolBar = addToolBar(tr("&Main"));
	pToolBar->addAction(Action_About);
	pToolBar->addAction(Action_Line);
	pToolBar->addAction(Action_Rect);
	pToolBar->addAction(Action_Ellipse);
	pToolBar->addAction(Action_Polygon);
	pToolBar->addAction(Action_Freehand);
	pToolBar->addAction(Action_Color);
	pToolBar->addAction(Action_Undo);
}

void MiniDraw::Creat_Menu() {
	pMenu = menuBar()->addMenu(tr("&Figure Tool"));
	pMenu->addAction(Action_About);
	pMenu->addAction(Action_Line);
	pMenu->addAction(Action_Rect);
	pMenu->addAction(Action_Ellipse);
	pMenu->addAction(Action_Polygon);
	pMenu->addAction(Action_Freehand);
	pMenu->addAction(Action_Color);
	pMenu->addAction(Action_Undo);
}

void MiniDraw::AboutBox() {
	QMessageBox::about(this, tr("About"), tr("MiniDraw"));
}

MiniDraw::~MiniDraw() {}