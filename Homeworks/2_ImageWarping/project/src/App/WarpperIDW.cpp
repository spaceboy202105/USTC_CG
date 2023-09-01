#include "WarpperIDW.h"

WarpperIDW::WarpperIDW()
{
	windows_height = 0; windows_width = 0;
	mu = -2;
	weight_sum = 0;
}

WarpperIDW::WarpperIDW(int w, int h, QVector<QPoint>p, QVector<QPoint>q)
{
	windows_height = h; windows_width = w;
	mu = -2;
	weight_sum = 0;
	src_list = QVector<QPoint>(p);
	tar_list = QVector<QPoint>(q);
	for (int i = 0; i < p.size(); i++)
	{
		src_list[i] = QPoint(src_list[i].x() - windows_width, src_list[i].y() - windows_height);
		tar_list[i] = QPoint(tar_list[i].x() - windows_width, tar_list[i].y() - windows_height);
	}
	T.resize(src_list.size(), 4);
	generate_t();
}

//WarpperIDW::WarpperIDW(Warpper wrapping)
//{
//	this->src_list = wrapping.src_list;
//	this->tar_list = wrapping.tar_list;
//	this->windows_width = wrapping.windows_width;
//	this->windows_height = wrapping.windows_height;
//}

WarpperIDW::~WarpperIDW()
{
}

//void WarpperIDW::init_vector(std::vector<QPoint>p, std::vector<QPoint>q)
//{
//
//}

QPoint WarpperIDW::ConvertPoint(QPoint point)
{
	Eigen::Vector2d convert_vec;
	Eigen::Vector2d vec_p;
	convert_vec.setZero();
	vec_p << point.x(), point.y();
	generate_w(point);
	if (W.empty())return point;
	else
	{
		for (int i = 0; i < W.size(); i++)
		{
			//Eigen::Matrix2 tmpT = T.row(i).reshaped(2, 2);
			Eigen::MatrixXd tmpT(2, 2);
			//tmpT.resize(2, 2);
			tmpT << T(i, 0), T(i, 1),
				T(i, 2), T(i, 3);
			Eigen::Vector2d vec_src_i;
			Eigen::Vector2d vec_tar_i;
			vec_src_i << src_list[i].x(), src_list[i].y();
			vec_tar_i << tar_list[i].x(), tar_list[i].y();
			convert_vec += (W[i] / weight_sum) * (vec_tar_i + tmpT * (vec_p - vec_src_i));
		}
	}
	return QPoint(convert_vec.x(), convert_vec.y());
}

void WarpperIDW::generate_t()
{
	if (src_list.size() == 1)
	{
		T << 1, 0,
			0, 1;
	}
	else if (src_list.size() == 2)//Singular matrix
	{
		for (int i = 0; i < 2; i++)
		{
			T(i, 0) = (tar_list[1].x() - tar_list[0].x()) / (src_list[1].x() - src_list[0].x());
			T(i, 1) = 0;
			T(i, 2) = 0;
			T(i, 3) = (tar_list[1].y() - tar_list[0].y()) / (src_list[1].y() - src_list[0].y());
		}
	}
	else
	{
		for (int i = 0; i < src_list.size(); i++)
		{
			Eigen::MatrixXd A(2, 2), B(2, 2), tmpT(2, 2);
			A.setZero(); B.setZero();
			for (int j = 0; j < src_list.size(); j++)
			{
				if (i != j)
				{
					double sigma = pow(distance(src_list[i], src_list[j]), mu);
					Eigen::Matrix2d diff_src;
					Eigen::Matrix2d diff_tar;
					diff_src << (double)src_list[j].x() - (double)src_list[i].x(), 0,
						(double)src_list[j].y() - (double)src_list[i].y(), 0;
					diff_tar << (double)tar_list[j].x() - (double)tar_list[i].x(), (double)tar_list[j].y() - (double)tar_list[i].y(),
						0, 0;
					A += sigma * diff_src * diff_src.transpose();
					B += sigma * diff_src * diff_tar;
				}
				tmpT = (A.inverse()) * B;
				T(i, 0) = tmpT(0, 0);
				T(i, 1) = tmpT(0, 1);
				T(i, 2) = tmpT(1, 0);
				T(i, 3) = tmpT(1, 1);
			}
		}
	}
}

void WarpperIDW::generate_w(QPoint p)
{
	weight_sum = 0;
	W.clear();
	for (int i = 0; i < src_list.size(); ++i)
	{
		W.push_back(pow(distance(src_list[i], p), mu));
		weight_sum += W[i];
	}
}