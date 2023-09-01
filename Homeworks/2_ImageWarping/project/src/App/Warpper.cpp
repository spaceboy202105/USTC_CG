#include"Warpper.h"
#include <chrono>
#include <iostream>

double Warpper::distance(QPoint p, QPoint q)
{
	double dist = ((double)p.x() - (double)q.x()) * ((double)p.x() - (double)q.x())
		+ ((double)p.y() - (double)q.y()) * ((double)p.y() - (double)q.y());
	return sqrt(dist);
}

void Warpper::output_wrapping_list()
{
}
void Warpper::load_wrapping_list()
{
}

Eigen::MatrixXd Warpper::ImageWrapping(QImage& image)
{
	auto start = std::chrono::high_resolution_clock::now();
	Eigen::MatrixXd mask(image.width(), image.height());
	mask.setZero();
	QImage image_bak(image);
	QPoint res_point;
	for (int i = 0; i < image.width(); ++i)
	{
		for (int j = 0; j < image.height(); ++j)
		{
			image.setPixel(i, j, qRgb(255, 255, 255));
		}
	}
	for (int i = 0; i < image.width(); ++i)
	{
		for (int j = 0; j < image.height(); ++j)
		{
			res_point = ConvertPoint(QPoint(i, j));
			if (res_point.x() > 0 && res_point.x() < image.width())
			{
				if (res_point.y() > 0 && res_point.y() < image.height())
				{
					image.setPixel(res_point, image_bak.pixel(i, j));
					mask(res_point.x(), res_point.y());
				}
			}
		}
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Execution time: " << duration.count() << "ms" << std::endl;
	return mask;
}

Warpper::Warpper()
{
	//src_list = std::vector<QPoint>();
	//tar_list = std::vector<QPoint>();
	//windows_width = 0;
	//windows_height = 0;
}

Warpper::~Warpper()
{
}