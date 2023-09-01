#pragma once
#include <QWidget>
#include "Warpper.h"
QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE
#include "WarpperList.h"
class ImageWidget :
	public QWidget
{
	Q_OBJECT

public:
	ImageWidget(void);
	~ImageWidget(void);

protected:
	void paintEvent(QPaintEvent* paintevent);

public slots:
	// File IO
	void Open();												// Open an image file, support ".bmp, .png, .jpg" format
	void Save();												// Save image to current file
	void SaveAs();												// Save image to another file

	// Image processing
	void Invert();												// Invert pixel value in image
	void Mirror(bool horizontal = false, bool vertical = true);		// Mirror image vertically or horizontally
	void TurnGray();											// Turn image to gray-scale map
	void Restore();												// Restore image to origin
	void Undo();
	void Choose_Control();
	void WrappingIDW_slot();
	void WrappingRBF_slot();
	void FixHole();
	//mouse event
	//how to choose points`pair:
	//using methods like draw line in lab1
	void mousePressEvent(QMouseEvent* mouseevent);
	void mouseMoveEvent(QMouseEvent* mouseevent);
	void mouseReleaseEvent(QMouseEvent* mouseevent);
private:
	QImage* ptr_image_;				// image
	QImage* ptr_image_backup_;
	QVector<QImage>stack_;
	//for undo;when restore ,it clear
	Warpper* warpper_;
	WarpperList* warpper_list_;
	bool warpping_status;
	bool draw_status;
	QPoint start_pt;
	QPoint end_pt;
	QVector<QLine>warpping_path_list_;
	Eigen::MatrixXd mask;
};
