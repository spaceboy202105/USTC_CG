#include "ScanLine.h"

ScanLine::ScanLine()
{
	m_ptr_edges = nullptr;
}

ScanLine::~ScanLine()
{
	delete[]m_ptr_edges;
	m_ptr_edges = nullptr;
}

QPoint ScanLine::get_start()
{
	return QPoint(x_st, y_st);
}

QPoint ScanLine::get_end()
{
	return QPoint(x_ed, y_ed);
}

void ScanLine::init_bounding_mask(QPoint st, QPoint ed)
{
	x_min = x_st = std::min(st.x(), ed.x());
	y_min = y_st = std::min(st.y(), ed.y());
	x_max = x_ed = std::max(st.x(), ed.x());
	y_max = y_ed = std::max(st.y(), ed.y());
	mask_width = abs(st.x() - ed.x()) + 1;
	mask_height = abs(st.y() - ed.y()) + 1;
	edge_points.push_back(QPoint(st.x(), st.y()));
	edge_points.push_back(QPoint(st.x(), ed.y()));
	edge_points.push_back(QPoint(ed.x(), ed.y()));
	edge_points.push_back(QPoint(ed.x(), st.y()));
	inside_mask_.resize(mask_height, mask_width);
	inside_mask_.setZero();
	points_size = edge_points.size();
}
void ScanLine::init_bounding_mask(QPainterPath path)
{
	init_bounding_mask(path.toFillPolygon().toPolygon());
}

void ScanLine::init_bounding_mask(QPolygon polygon)
{
	auto bounding_rect = polygon.boundingRect();
	y_min = bounding_rect.top(), y_max = bounding_rect.bottom();
	x_max = bounding_rect.right(), x_min = bounding_rect.left();
	// std::cout << y_min << " " << y_max;
	mask_width = x_max - x_min + 1;
	mask_height = y_max - y_min + 1;
	x_st = x_min;
	y_st = y_min;
	x_ed = x_max;
	y_ed = y_max;
	inside_mask_.resize(mask_height, mask_width);
	inside_mask_.setZero();
	edge_points = polygon;
	edge_points.pop_back();//the last one == the first one(in a closed polygon)
	points_size = edge_points.size();
}

void ScanLine::initEdge()
{
	inside_mask_.setZero();
	if (m_ptr_edges)
	{
		delete[] m_ptr_edges; m_ptr_edges = nullptr;
	}
	m_ptr_edges = new edge[points_size];

	for (int i = 0; i < points_size - 1; i++)
	{
		if (edge_points[i].y() > edge_points[i + 1].y())
		{
			m_ptr_edges[i].h_pt = edge_points[i];
			m_ptr_edges[i].l_pt = edge_points[i + 1];
		}
		else
		{
			m_ptr_edges[i].h_pt = edge_points[i + 1];
			m_ptr_edges[i].l_pt = edge_points[i];
		}
		m_ptr_edges[i].y_max = m_ptr_edges[i].h_pt.y();
		m_ptr_edges[i].x = m_ptr_edges[i].l_pt.x();
		if (m_ptr_edges[i].h_pt.y() == m_ptr_edges[i].l_pt.y())
			m_ptr_edges[i].dx = DBL_MAX;
		else
			m_ptr_edges[i].dx = ((double)m_ptr_edges[i].h_pt.x() - (double)m_ptr_edges[i].l_pt.x()) / ((double)m_ptr_edges[i].h_pt.y() - (double)m_ptr_edges[i].l_pt.y());
	}

	if (edge_points[0].y() > edge_points[points_size - 1].y())
	{
		m_ptr_edges[points_size - 1].h_pt = edge_points[0];
		m_ptr_edges[points_size - 1].l_pt = edge_points[points_size - 1];
	}
	else
	{
		m_ptr_edges[points_size - 1].h_pt = edge_points[points_size - 1];
		m_ptr_edges[points_size - 1].l_pt = edge_points[0];
	}

	m_ptr_edges[points_size - 1].y_max = m_ptr_edges[points_size - 1].h_pt.y();
	m_ptr_edges[points_size - 1].x = m_ptr_edges[points_size - 1].l_pt.x();
	if (m_ptr_edges[points_size - 1].h_pt.y() == m_ptr_edges[points_size - 1].l_pt.y())
		m_ptr_edges[points_size - 1].dx = DBL_MAX;
	else
		m_ptr_edges[points_size - 1].dx = ((double)m_ptr_edges[points_size - 1].h_pt.x() - (double)m_ptr_edges[points_size - 1].l_pt.x()) / ((double)m_ptr_edges[points_size - 1].h_pt.y() - (double)m_ptr_edges[points_size - 1].l_pt.y());
}

void ScanLine::SortNET()
{
	// sort edge according to x_min
	for (int scan_line = y_min; scan_line < y_max; scan_line++)
	{
		int index = scan_line - y_min;
		int sz = NET[index].size();
		for (int i = 0; i < sz - 1; i++)
		{
			for (int k = i + 1; k < sz; k++)
			{
				if (NET[index][i].x > NET[index][k].x)
				{
					edge e = NET[index][i];
					NET[index][i] = NET[index][k];
					NET[index][k] = e;
				}
			}
		}
	}
}

void ScanLine::initNET()
{
	NET.clear();

	for (int i = 0; i < y_max - y_min + 1; i++)
	{
		NET.append(QVector<edge>(0));
	}
	//	add new edge to NET
	for (int i = 0; i < points_size; i++)
	{
		int scanline = m_ptr_edges[i].l_pt.y() - y_min;
		NET[scanline].push_back(m_ptr_edges[i]);
	}
}

void ScanLine::initAET()
{
	AET.clear();
	for (int i = 0; i < y_max - y_min + 1; i++)
	{
		AET.append(QVector<edge>(0));
	}
}

void ScanLine::generate_inside_mask()
{
	initEdge();
	initNET();
	SortNET();
	initAET();
	FillPixel();
}

// Fill Polygon
void ScanLine::FillPixel()
{
	cv::Mat img_mask(mask_height + 1, mask_width + 1, CV_8UC1, cv::Scalar(255));
	for (int scan_line = y_min; scan_line < y_max; scan_line++)
	{
		int index = scan_line - y_min;
		for (int i = 0; i < NET[index].size(); i++)
		{
			AET[index].push_back(NET[index][i]);
		}
		for (int i = 0; i < AET[index].size(); i++)
		{
			if (scan_line == AET[index][i].y_max)
			{
				AET[index].remove(i);
				i--;
			}
		}
		int sz = AET[index].size();
		for (int i = 0; i < sz - 1; i++)
		{
			for (int k = i + 1; k < sz; k++)
			{
				if (AET[index][i].x > AET[index][k].x)
				{
					edge e = AET[index][i];
					AET[index][i] = AET[index][k];
					AET[index][k] = e;
				}
			}
		}
		for (int i = 0; i < AET[index].size() - 1; i += 2)
		{
			for (int x0 = AET[index][i].x; x0 < (int)(AET[index][i + 1].x); x0++)
			{
				inside_mask_(index, x0 - x_min) = 1;
			}
		}
		sz = AET[index].size();
		for (int i = 0; i < sz; i++)
		{
			AET[index][i].x += AET[index][i].dx;
			AET[index + 1].push_back(AET[index][i]);
		}
	}
}