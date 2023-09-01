#include "WarpperList.h"
void WarpperList::put_pair(QPoint src, QPoint tar)
{
	src_list.push_back(src);
	tar_list.push_back(tar);
}

QVector<QVector<QPoint>> WarpperList::get_pair_list()
{
	auto res = QVector<QVector<QPoint>>({ src_list,tar_list });
	return res;
}

void WarpperList::set_pair_list(QVector<QPoint> p, QVector<QPoint> q)
{
	src_list = p;
	tar_list = q;
}

WarpperList::WarpperList()
{
	src_list = QVector<QPoint>();
	tar_list = QVector<QPoint>();
}

WarpperList::~WarpperList()
{
}

void WarpperList::undo_choose()
{
	src_list.pop_back();
	tar_list.pop_back();
}

void WarpperList::clear_choose()
{
	src_list.clear(); tar_list.clear();
}