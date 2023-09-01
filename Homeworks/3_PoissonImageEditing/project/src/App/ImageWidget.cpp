#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets>
#include <iostream>
#include "ChildWindow.h"

using std::cout;
using std::endl;

ImageWidget::ImageWidget(ChildWindow* relatewindow)
{
	draw_status_ = kNone;
	is_choosing_ = false;
	is_pasting_ = false;

	point_start_ = QPoint(0, 0);
	point_end_ = QPoint(0, 0);

	source_window_ = nullptr;
	shape = nullptr;
	scanline = nullptr;
	poisson = nullptr;

	paste_status_ = kNORMAL;
	inside_mask_.resize(0, 0);
}

ImageWidget::~ImageWidget(void)
{
	img.release();
	img_backup.release();
	img_last.release();
	if (shape != nullptr)
	{
		delete shape;
	}
	if (scanline != nullptr)
	{
		delete scanline;
	}
	if (poisson != nullptr)
	{
		delete poisson;
	}
}

int ImageWidget::ImageWidth()
{
	return img.cols;
}

int ImageWidget::ImageHeight()
{
	return img.rows;
}

void ImageWidget::set_draw_status_to_choose()
{
	draw_status_ = kChoose;
}

void ImageWidget::set_draw_status_to_paste()
{
	draw_status_ = kPaste;
}

void ImageWidget::set_mixing_paste()
{
	paste_status_ = kMIXING;
}

void ImageWidget::set_normal_paste()
{
	paste_status_ = kNORMAL;
}

void ImageWidget::set_poisson_paste()
{
	paste_status_ = kPOISSON;
}

const cv::Mat& ImageWidget::image()
{
	return img;
}

void ImageWidget::set_source_window(ChildWindow* childwindow)
{
	source_window_ = childwindow;
}

void ImageWidget::paintEvent(QPaintEvent* paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QImage image_((unsigned char*)(img.data), img.cols, img.rows, img.step, QImage::Format_RGB888);
	QRect rect = QRect(0, 0, image_.width(), image_.height());
	painter.drawImage(rect, image_);

	// Draw choose region
	painter.setBrush(Qt::NoBrush);
	painter.setPen(Qt::red);
	if (shape != NULL)
	{
		shape->Draw(painter);//just like hw1
	}

	painter.end();
}

void ImageWidget::mousePressEvent(QMouseEvent* mouseevent)
{
	if (Qt::LeftButton == mouseevent->button())
	{
		switch (draw_status_)
		{
		case kChoose:
			is_choosing_ = true;
			point_start_ = point_end_ = mouseevent->pos();
			shape->set_start(point_start_);
			shape->set_end(point_end_);
			if (shape->type_ == CFigure::kPolygon)
			{
				shape->update(1);
			}
			break;

		case kPaste:
		{
			is_pasting_ = true;

			// Start point in object image
			int xpos = mouseevent->pos().rx();
			int ypos = mouseevent->pos().ry();

			// Start point in source image
			int xsourcepos = source_window_->imagewidget_->scanline->get_start().rx();
			int ysourcepos = source_window_->imagewidget_->scanline->get_start().ry();

			// Width and Height of rectangle region
			int w = source_window_->imagewidget_->scanline->get_end().rx();
			-source_window_->imagewidget_->scanline->get_start().rx() + 1;
			int h = source_window_->imagewidget_->scanline->get_end().ry()
				- source_window_->imagewidget_->scanline->get_start().ry() + 1;
			inside_mask_ = source_window_->imagewidget_->inside_mask_;

			// Paste
			if ((xpos + w < img.cols) && (ypos + h < img.rows))
			{
				switch (paste_status_)
				{
				case kMIXING:
					source_window_->imagewidget_->poisson->paste_mixing_poisson(mouseevent->pos(), source_window_->imagewidget_->scanline->get_start(),
						img, source_window_->imagewidget_->img);
					break;
				case kPOISSON:
					source_window_->imagewidget_->poisson->paste_poisson(mouseevent->pos(), source_window_->imagewidget_->scanline->get_start(),
						img, source_window_->imagewidget_->img);
					break;
				case kNORMAL:
					source_window_->imagewidget_->poisson->paste_normal(mouseevent->pos(), source_window_->imagewidget_->scanline->get_start(),
						img, source_window_->imagewidget_->img);
					break;
				default:
					break;
				}
			}
		}
		update();
		break;
		default:
			break;
		}
	}
	if (Qt::RightButton == mouseevent->button())
	{
		if (draw_status_ == kChoose && shape->type_ == CFigure::kPolygon)
		{
			is_choosing_ = false;
			draw_status_ = kNone;
			scanline = new ScanLine;
			poisson = new ImagePaste;
			shape->update(0);
			scanline->init_bounding_mask(shape->get_points());
			scanline->generate_inside_mask();
			poisson->set_inside_mask(scanline->inside_mask_);
			poisson->poisson_init(img);

			update();
		}
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
	case kChoose:
		// Store point position for rectangle region
		if (is_choosing_)
		{
			point_end_ = mouseevent->pos();
			shape->set_end(point_end_);
		}
		break;

	case kPaste:
		// Paste rectangle region to object image
		if (is_pasting_)
		{
			// Start point in object image
			int xpos = mouseevent->pos().rx();
			int ypos = mouseevent->pos().ry();

			// Start point in source image
			int xsourcepos = source_window_->imagewidget_->scanline->get_start().rx();
			int ysourcepos = source_window_->imagewidget_->scanline->get_start().ry();

			// Width and Height of rectangle region
			int w = source_window_->imagewidget_->scanline->get_end().rx()
				- source_window_->imagewidget_->scanline->get_start().rx() + 1;
			int h = source_window_->imagewidget_->scanline->get_end().ry()
				- source_window_->imagewidget_->scanline->get_start().ry() + 1;

			// Paste
			if ((xpos > 0) && (ypos > 0) && (xpos + w < img.cols) && (ypos + h < img.rows))
			{
				// Restore image
				img = img_last.clone();
				// Paste
				switch (paste_status_)
				{
				case kMIXING:
					source_window_->imagewidget_->poisson->paste_mixing_poisson(mouseevent->pos(), source_window_->imagewidget_->scanline->get_start(),
						img, source_window_->imagewidget_->img);
					break;
				case kPOISSON:
					source_window_->imagewidget_->poisson->paste_poisson(mouseevent->pos(), source_window_->imagewidget_->scanline->get_start(),
						img, source_window_->imagewidget_->img);
					break;
				case kNORMAL:
					source_window_->imagewidget_->poisson->paste_normal(mouseevent->pos(), source_window_->imagewidget_->scanline->get_start(),
						img, source_window_->imagewidget_->img);
					break;
				default:
					break;
				}
			}
		}
	default:
		break;
	}
	update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
	case kChoose:
		if (is_choosing_)
		{
			point_end_ = mouseevent->pos();
			if (shape->type_ == CFigure::kRect)
			{
				scanline = new ScanLine;
				poisson = new ImagePaste;
				is_choosing_ = false;
				draw_status_ = kNone;
				scanline->init_bounding_mask(point_start_, point_end_);
				scanline->generate_inside_mask();
				poisson->set_inside_mask(scanline->inside_mask_);
				poisson->poisson_init(img);
			}
			if (shape->type_ == CFigure::kFreedraw)
			{
				shape->set_end(point_start_);
				scanline = new ScanLine;
				poisson = new ImagePaste;
				is_choosing_ = false;
				draw_status_ = kNone;
				scanline->init_bounding_mask(shape->get_path());
				scanline->generate_inside_mask();
				poisson->set_inside_mask(scanline->inside_mask_);
				poisson->poisson_init(img);
			}
		}

	case kPaste:
		if (is_pasting_)
		{
			is_pasting_ = false;
			draw_status_ = kNone;
			img_last = img.clone();
		}

	default:
		break;
	}

	update();
}

void ImageWidget::Open(QString filename)
{
	// Load file
	if (!filename.isEmpty())
	{
		img = cv::imread(filename.toStdString());
		cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
		img_backup = img.clone();
		img_last = img.clone();
		std::cout << img.cols << " " << img.rows << " " << img.step << std::endl;
	}
	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}

	//image_->save(filename);
	cv::Mat img_save = img.clone();
	cv::cvtColor(img_save, img_save, cv::COLOR_RGB2BGR);
	cv::imwrite(filename.toStdString(), img_save);
}

void ImageWidget::Invert()
{
	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			img.at<cv::Vec3b>(i, j)[0] = 255 - img.at<cv::Vec3b>(i, j)[0];
			img.at<cv::Vec3b>(i, j)[1] = 255 - img.at<cv::Vec3b>(i, j)[1];
			img.at<cv::Vec3b>(i, j)[2] = 255 - img.at<cv::Vec3b>(i, j)[2];
		}
	}
	img_last = img.clone();
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	cv::flip(img, img, 1);
	img_last = img.clone();
	update();
}

void ImageWidget::TurnGray()
{
	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			unsigned char gray = (img.at<cv::Vec3b>(i, j)[0] + img.at<cv::Vec3b>(i, j)[1] + img.at<cv::Vec3b>(i, j)[2]) / 3;
			img.at<cv::Vec3b>(i, j) = cv::Vec3b(gray, gray, gray);
		}
	}
	img_last = img.clone();
	update();
}

void ImageWidget::Restore()
{
	if (shape != NULL)
	{
		delete shape;
		shape = NULL;
	}

	img = img_backup.clone();
	img_last = img_backup.clone();
	point_start_ = point_end_ = QPoint(0, 0);
	update();
}