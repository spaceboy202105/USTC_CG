#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets>
#include <iostream>

#include "WarpperIDW.h"
#include "WarpperRBF.h"
using std::cout;
using std::endl;

ImageWidget::ImageWidget(void)
{
	stack_ = QVector<QImage>();
	ptr_image_ = new QImage();
	ptr_image_backup_ = new QImage();
	warpping_status = false;
	warpper_list_ = new WarpperList();
	warpping_path_list_ = QVector<QLine>();
	draw_status = false;
	start_pt = QPoint();
	end_pt = QPoint();
}

ImageWidget::~ImageWidget(void)
{
}

void ImageWidget::paintEvent(QPaintEvent* paintevent)//paint image and background
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect((width() - ptr_image_->width()) / 2, (height() - ptr_image_->height()) / 2, ptr_image_->width(), ptr_image_->height());
	painter.drawImage(rect, *ptr_image_);
	QPen pen;
	for (QLine wrapping_path_list : warpping_path_list_)
	{
		pen.setWidth(3);
		pen.setColor(Qt::black);
		painter.setPen(pen);
		painter.drawLine(wrapping_path_list);
		pen.setColor(Qt::green);
		pen.setWidth(4);
		painter.setPen(pen);
		painter.drawPoint(wrapping_path_list.p1());
		pen.setColor(Qt::red);
		pen.setWidth(4);
		painter.setPen(pen);
		painter.drawPoint(wrapping_path_list.p2());
	}
	update();
	painter.end();
}

void ImageWidget::Open()//load image
{
	warpping_status = false;

	// Open file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Read Image"), ".", tr("Images(*.bmp *.png *.jpg)"));

	// Load file
	if (!fileName.isEmpty())
	{
		ptr_image_->load(fileName);
		*(ptr_image_backup_) = *(ptr_image_);
	}
	stack_.clear();
	//stack_.push_back(*ptr_image_);

	//ptr_image_->invertPixels(QImage::InvertRgb);
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	//*(ptr_image_) = ptr_image_->rgbSwapped();
	cout << "image size: " << ptr_image_->width() << ' ' << ptr_image_->height() << endl;
	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	warpping_status = false;

	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}

	ptr_image_->save(filename);
}

void ImageWidget::Invert()
{
	warpping_path_list_.clear();
	warpping_status = false;
	stack_.push_back(*ptr_image_);

	for (int i = 0; i < ptr_image_->width(); i++)
	{
		for (int j = 0; j < ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			ptr_image_->setPixel(i, j, qRgb(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color)));
		}
	}
	// equivalent member function of class QImage
	// ptr_image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	QImage image_tmp(*(ptr_image_));
	int width = ptr_image_->width();
	int height = ptr_image_->height();
	warpping_path_list_.clear();
	warpping_status = false;
	stack_.push_back(*ptr_image_);

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, height - 1 - j));
				}
			}
		}
		else			//仅水平翻转
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, j));
				}
			}
		}
	}
	else
	{
		if (isvertical)		//仅垂直翻转
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(i, height - 1 - j));
				}
			}
		}
	}
	// equivalent member function of class QImage
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray()
{
	warpping_path_list_.clear();
	warpping_status = false;
	stack_.push_back(*ptr_image_);

	for (int i = 0; i < ptr_image_->width(); i++)
	{
		for (int j = 0; j < ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			int gray_value = (qRed(color) + qGreen(color) + qBlue(color)) / 3;
			ptr_image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value));
		}
	}
	update();
}

void ImageWidget::Restore()
{
	*(ptr_image_) = *(ptr_image_backup_);
	warpping_status = false;
	warpping_path_list_.clear();
	stack_.clear();
	update();
}

void ImageWidget::Undo()
{
	if (warpping_status && !warpping_path_list_.empty())
	{
		warpping_path_list_.pop_back();
		warpper_list_->undo_choose();
		return;
	}
	if (!stack_.empty())
	{
		ptr_image_ = new QImage(stack_.back());
		stack_.pop_back();
	}
}

void ImageWidget::Choose_Control()
{
	warpper_list_->clear_choose();//clear the previous choosing points_pair
	warpping_path_list_.clear();
	warpping_status = true;
}

void ImageWidget::WrappingIDW_slot()
{
	stack_.push_back(*ptr_image_);

	if (warpper_list_->get_pair_list()[0].empty())return;
	warpping_path_list_.clear();
	warpping_status = false;
	auto tmp_vec = warpper_list_->get_pair_list();
	warpper_ = new WarpperIDW((width() - ptr_image_->width()) / 2, (height() - ptr_image_->height()) / 2, tmp_vec[0], tmp_vec[1]);
	//warpper_->init_vector();

	mask = warpper_->ImageWrapping(*ptr_image_);

	warpper_list_->clear_choose();
}

void ImageWidget::WrappingRBF_slot()
{
	stack_.push_back(*ptr_image_);
	if (warpper_list_->get_pair_list()[0].empty())return;
	warpping_path_list_.clear();
	warpping_status = false;
	auto tmp_vec = warpper_list_->get_pair_list();
	warpper_ = new WarpperRBF((width() - ptr_image_->width()) / 2, (height() - ptr_image_->height()) / 2, tmp_vec[0], tmp_vec[1]);
	//warpper_->init_vector();

	mask = warpper_->ImageWrapping(*ptr_image_);

	warpper_list_->clear_choose();
}

void ImageWidget::FixHole()
{
	stack_.push_back(*ptr_image_);
	warpping_path_list_.clear();
	warpping_status = false;
}

void ImageWidget::mousePressEvent(QMouseEvent* mouseevent)//choose the from_position
{
	if (warpping_status && Qt::LeftButton == mouseevent->button())
	{
		draw_status = true;
		start_pt = end_pt = mouseevent->pos();
		update();
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent* mouseevent)//update to_position
{
	if (warpping_status && draw_status)
	{
		end_pt = mouseevent->pos();
		update();
	}
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* mouseevent)
{
	if (warpping_status && draw_status)
	{
		end_pt = mouseevent->pos();
		warpper_list_->put_pair(start_pt, end_pt);
		QLine line = QLine(start_pt, end_pt);
		warpping_path_list_.push_back(line);

		draw_status = false;
		update();
	}
}