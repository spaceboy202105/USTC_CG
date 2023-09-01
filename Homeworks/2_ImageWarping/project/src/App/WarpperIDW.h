#pragma once
#include "Warpper.h"
class WarpperIDW :public Warpper
{
public:
	WarpperIDW();
	WarpperIDW(int  w, int h, QVector<QPoint>p, QVector<QPoint>q);
	//WarpperIDW(Warpper wrapping);//copy constructor
	~WarpperIDW();
	//void init_vector(std::vector<QPoint>p, std::vector<QPoint>q)override;
	QPoint ConvertPoint(QPoint point) override;

private:
	void generate_t();
	void generate_w(QPoint p);
	Eigen::MatrixXd T;
	QVector<double>W;
};