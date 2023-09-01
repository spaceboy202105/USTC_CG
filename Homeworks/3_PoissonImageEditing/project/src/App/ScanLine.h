#pragma once
# include <QtGui>
#include<eigen3/Eigen/Dense>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <cfloat>

typedef struct edge
{
	QPoint h_pt;
	QPoint l_pt;
	double x;
	double dx;
	double y_max;
}edge;

class ScanLine
{
public:
	ScanLine();
	~ScanLine();

public:
	void init_bounding_mask(QPoint st, QPoint ed);
	void init_bounding_mask(QPainterPath path);
	void init_bounding_mask(QPolygon polygon);
	void initEdge();
	void SortNET();
	void initNET();
	void initAET();
	void generate_inside_mask();	//	Get inside mask
	QPoint get_start();
	QPoint get_end();

private:

	void FillPixel();

public:
	Eigen::MatrixXi inside_mask_;

private:
	QVector <QPoint> edge_points;
	edge* m_ptr_edges;
	int points_size;
	QVector<QVector<edge>> NET;
	QVector<QVector<edge>> AET;
	int x_min, x_max, y_min, y_max;
	int mask_width, mask_height;
	int x_st, y_st, x_ed, y_ed;
};