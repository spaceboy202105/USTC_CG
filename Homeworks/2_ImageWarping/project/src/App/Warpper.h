//choose the pair of point`s list
#ifndef WRAPPING_H
#define WRAPPING_H
#include <QPointF>
#include <vector>
#include "D://vcpkg/installed/x64-windows/include/Eigen/Dense"
#include <QImage>

//#include "ImageWidget.h"

class Warpper
{
public:
	QVector<QPoint>src_list;//from list
	QVector<QPoint>tar_list;//to list

public:

	double distance(QPoint p, QPoint q);
	void output_wrapping_list();
	void load_wrapping_list();
	//virtual void init_vector(std::vector<QPoint>p, std::vector<QPoint>q) = 0;
	virtual QPoint ConvertPoint(QPoint point) = 0;//each wrapping methods definitionvoid undo_choose();
	Eigen::MatrixXd ImageWrapping(QImage& image);
	Warpper();
	virtual  ~Warpper();
	//Warpper(int w, int h) :windows_width(w), windows_height(h) {
	//	src_list = std::vector<QPoint>();
	//	tar_list = std::vector<QPoint>();
	//}
	int windows_width;
	int windows_height;
	double weight_sum;
	double mu;
};

#endif
;