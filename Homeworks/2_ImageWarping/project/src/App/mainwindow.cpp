#include "mainwindow.h"
#include <QtWidgets>
#include <QImage>
#include <QPainter>
#include "ImageWidget.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	//ui.setupUi(this);

	setGeometry(300, 150, 800, 450);

	imagewidget_ = new ImageWidget();
	setCentralWidget(imagewidget_);

	CreateActions();
	CreateMenus();
	CreateToolBars();
	CreateStatusBar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent* e)
{
}

void MainWindow::paintEvent(QPaintEvent* paintevent)
{
}

void MainWindow::CreateActions()
{
	action_new_ = new QAction(QIcon(":/MainWindow/Resources/images/new.jpg"), tr("&New"), this);
	action_new_->setShortcut(QKeySequence::New);
	action_new_->setStatusTip(tr("Create a new file"));
	// connect ...

	action_open_ = new QAction(QIcon(":/MainWindow/Resources/images/open.jpg"), tr("&Open..."), this);
	action_open_->setShortcuts(QKeySequence::Open);
	action_open_->setStatusTip(tr("Open an existing file"));
	connect(action_open_, &QAction::triggered, imagewidget_, &ImageWidget::Open);

	action_save_ = new QAction(QIcon(":/MainWindow/Resources/images/save.jpg"), tr("&Save"), this);
	action_save_->setShortcuts(QKeySequence::Save);
	action_save_->setStatusTip(tr("Save the document to disk"));
	// connect ...

	action_saveas_ = new QAction(tr("Save &As..."), this);
	action_saveas_->setShortcuts(QKeySequence::SaveAs);
	action_saveas_->setStatusTip(tr("Save the document under a new name"));
	connect(action_saveas_, &QAction::triggered, imagewidget_, &ImageWidget::SaveAs);

	action_invert_ = new QAction(tr("Inverse"), this);
	action_invert_->setStatusTip(tr("Invert all pixel value in the image"));
	connect(action_invert_, &QAction::triggered, imagewidget_, &ImageWidget::Invert);

	action_mirror_ = new QAction(tr("Mirror"), this);
	action_mirror_->setStatusTip(tr("Mirror image vertically or horizontally"));
	// The slot requires more arguments than the signal provides.
	//connect(action_mirror_, &QAction::triggered, imagewidget_, &ImageWidget::Mirror);
	connect(action_mirror_, &QAction::triggered, [this]() {
		imagewidget_->Mirror(); // use default arguments
		});

	action_gray_ = new QAction(tr("Grayscale"), this);
	action_gray_->setStatusTip(tr("Gray-scale map"));
	connect(action_gray_, &QAction::triggered, imagewidget_, &ImageWidget::TurnGray);

	action_restore_ = new QAction(tr("Restore"), this);
	action_restore_->setStatusTip(tr("Show origin image"));
	connect(action_restore_, &QAction::triggered, imagewidget_, &ImageWidget::Restore);
	action_undo_ = new QAction(tr("Undo"), this);
	action_undo_->setStatusTip(tr("Undo your performance"));
	connect(action_undo_, &QAction::triggered, imagewidget_, &ImageWidget::Undo);

	action_wrapping_IDW_ = new QAction(tr("IDW"), this);
	action_wrapping_IDW_->setStatusTip(tr("Wrapping image by using IDW methods"));
	connect(action_wrapping_IDW_, &QAction::triggered, imagewidget_, &ImageWidget::WrappingIDW_slot);
	action_wrapping_RBF_ = new QAction(tr("RBF"), this);
	action_wrapping_RBF_->setStatusTip(tr("Wrapping image by using RBF methods"));
	connect(action_wrapping_RBF_, &QAction::triggered, imagewidget_, &ImageWidget::WrappingRBF_slot);

	action_fixhole_ = new QAction(tr("Fix"), this);
	action_fixhole_->setStatusTip(tr("Fixing hole in image"));
	connect(action_fixhole_, &QAction::triggered, imagewidget_, &ImageWidget::FixHole);

	action_choose_ = new QAction(tr("Wrapping Choose"), this);
	action_choose_->setStatusTip(tr("Choose wrapping points` pair"));
	connect(action_choose_, &QAction::triggered, imagewidget_, &ImageWidget::Choose_Control);
}

void MainWindow::CreateMenus()
{
	menu_file_ = menuBar()->addMenu(tr("&File"));
	menu_file_->setStatusTip(tr("File menu"));
	menu_file_->addAction(action_new_);
	menu_file_->addAction(action_open_);
	menu_file_->addAction(action_save_);
	menu_file_->addAction(action_saveas_);
	menu_file_->addAction(action_undo_);

	menu_edit_ = menuBar()->addMenu(tr("&Edit"));
	menu_edit_->setStatusTip(tr("Edit menu"));
	menu_edit_->addAction(action_invert_);
	menu_edit_->addAction(action_mirror_);
	menu_edit_->addAction(action_gray_);
	menu_edit_->addAction(action_choose_);
	menu_edit_->addAction(action_wrapping_IDW_);
	menu_edit_->addAction(action_wrapping_RBF_);
	menu_edit_->addAction(action_fixhole_);
	menu_edit_->addAction(action_restore_);
}

void MainWindow::CreateToolBars()
{
	toolbar_file_ = addToolBar(tr("File"));
	toolbar_file_->addAction(action_new_);
	toolbar_file_->addAction(action_open_);
	toolbar_file_->addAction(action_save_);
	toolbar_file_->addAction(action_undo_);

	// Add separator in toolbar
	toolbar_file_->addSeparator();
	toolbar_file_->addAction(action_invert_);
	toolbar_file_->addAction(action_mirror_);
	toolbar_file_->addAction(action_gray_);
	toolbar_file_->addAction(action_choose_);
	toolbar_file_->addAction(action_wrapping_IDW_);
	toolbar_file_->addAction(action_wrapping_RBF_);
	toolbar_file_->addAction(action_fixhole_);
	toolbar_file_->addAction(action_restore_);
}

void MainWindow::CreateStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}