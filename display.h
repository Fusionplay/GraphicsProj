#ifndef DISPLAY_H
#define DISPLAY_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QList>
#include "structs.h"


namespace Ui {
class display;
}

class display : public QWidget
{
	Q_OBJECT

public:
	explicit display(char *argv[], QWidget *parent = nullptr);
	~display();
	void paintEvent(QPaintEvent *event);


public slots:
	void draw();


private:
	Ui::display *ui;
	QPixmap *hehe;
	int w; //width, 宽度，x轴
	int h; //height, 高度，y轴
	QPen pen;

	QList<Item> item;

	char **args;

	void ini_pixmap();

	void draw_to_pixmap();
	void del_units();

	void drawLine(int id, int x1, int y1, int x2, int y2, char alg[]);
	void drawEllipse(int id, int x0, int y0, int a, int b);
	void drawPolygon(int id, int pnum, Point *dots, char *alg);
	void drawCurve(int id, int pnum, Point *dots, char *alg);

	void translate_unit(int id, int dx, int dy);
	void rotate_unit(int id, int x, int y, int r);
	void scale_unit(int id, int x, int y, double s);

	void clip_line(int id, int x1, int y1, int x2, int y2, char *alg); //线段裁剪

};

#endif // DISPLAY_H
