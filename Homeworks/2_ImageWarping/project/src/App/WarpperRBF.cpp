#include "WarpperRBF.h"

//void WarpperRBF::init_vector(std::vector<QPoint> p, std::vector<QPoint> q)
//{
//}

QPoint WarpperRBF::ConvertPoint(QPoint point)
{
	double dx = 0;
	double dy = 0;
	if (src_list.size() > 1)
	{
		for (int i = 0; i < src_list.size(); i++)
		{
			dx += w_x(i) * pow(pow(distance(src_list[i], point), 2) + radius[i] * radius[i], mu);
			dy += w_y(i) * pow(pow(distance(src_list[i], point), 2) + radius[i] * radius[i], mu);
		}
	}
	else
	{
		dx = (double)tar_list[0].x() - (double)src_list[0].x();
		dy = (double)tar_list[0].y() - (double)src_list[0].y();
	}
	return QPoint(dx + point.x(), dy + point.y());
}

WarpperRBF::WarpperRBF()
{
}

WarpperRBF::~WarpperRBF()
{
}

WarpperRBF::WarpperRBF(int w, int h, QVector<QPoint> p, QVector<QPoint> q)
{
	windows_height = h; windows_width = w;
	mu = 0.8;
	weight_sum = 0;
	src_list = QVector<QPoint>(p);
	tar_list = QVector<QPoint>(q);
	for (int i = 0; i < p.size(); i++)
	{
		src_list[i] = QPoint(src_list[i].x() - windows_width, src_list[i].y() - windows_height);
		tar_list[i] = QPoint(tar_list[i].x() - windows_width, tar_list[i].y() - windows_height);
	}
	if (src_list.size() <= 1)return;
	if (src_list.size() > 1)
	{
		for (int i = 0; i < src_list.size(); ++i)
		{
			int d_min = INT_MAX;
			for (int j = 0; j < src_list.size(); ++j)
			{
				if (i != j)
				{
					int dis = distance(src_list[i], src_list[j]);
					d_min = std::min(d_min, dis);
				}
			}
			radius.push_back(d_min);
		}
	}
	else
	{
		radius.push_back(0);
	}
	get_W();
}

void WarpperRBF::get_W()
{
	Eigen::MatrixXd A(src_list.size(), src_list.size());
	Eigen::VectorXd b_x(src_list.size()), b_y(src_list.size());
	A.setZero(), b_x.setZero(), b_y.setZero();
	if (src_list.size() > 1)
	{
		for (int i = 0; i < A.rows(); ++i)
		{
			for (int j = 0; j < A.cols(); ++j)
			{
				A(i, j) = pow(pow(distance(src_list[i], src_list[j]), 2) + radius[j] * radius[j], mu);
			}
			b_x(i) = (double)tar_list[i].x() - (double)src_list[i].x();
			b_y(i) = (double)tar_list[i].y() - (double)src_list[i].y();
		}
		w_x.resize(src_list.size()), w_y.resize(src_list.size());
		w_x = A.inverse() * b_x;
		w_y = A.inverse() * b_y;
	}
}