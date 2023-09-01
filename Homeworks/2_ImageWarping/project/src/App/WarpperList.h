#pragma once
#include <QPoint>
#include <QtGui/QtGui>

class WarpperList
{
public:
	WarpperList();
	~WarpperList();
	void undo_choose();
	void clear_choose();
	void put_pair(QPoint src, QPoint tar);
	QVector<QVector<QPoint>> get_pair_list();
	void set_pair_list(QVector<QPoint>p, QVector<QPoint>q);
private:
	QVector<QPoint>src_list;//from list
	QVector<QPoint>tar_list;//to list
};
