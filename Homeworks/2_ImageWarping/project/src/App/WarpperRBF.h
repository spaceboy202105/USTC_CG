#pragma once
#include"Warpper.h"
class WarpperRBF :public Warpper
{
public:
	//void init_vector(std::vector<QPoint>p, std::vector<QPoint>q) ;
	QPoint ConvertPoint(QPoint point) override;
	WarpperRBF();
	~WarpperRBF() override;
	WarpperRBF(int w, int h, QVector<QPoint>p, QVector<QPoint>q);
private:
	void get_W();
	Eigen::VectorXd w_x, w_y;
	QVector<double>radius;
};