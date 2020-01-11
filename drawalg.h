#ifndef DRAWALG_H
#define DRAWALG_H

#include "structs.h"

void line_DDA(QList<Point>* line, int x1, int y1, int x2, int y2);
void line_Bre(QList<Point>* line, int x1, int y1, int x2, int y2);

void ellipse_mid(QList<Point>* elli, int x0, int y0, int a, int b);

void translate(Item* it, int dx, int dy); //平移
void translate(QList<Point>* plist, int dx, int dy);
void rotate(Item* it, int x, int y, int r); //旋转
void scale(Item* it, int x, int y, double s); //缩放

bool clip_cohen(Item* it, int x1, int y1, int x2, int y2); //cohen裁剪算法
bool clip_liang(Item* it, int x1, int y1, int x2, int y2); //liang裁剪算法

void Bezier(QList<Point>* plist, int pnum, QList<Point> *dots);
void Bspline(QList<Point>* plist, int pnum, QList<Point> *dots);

#endif // DRAWALG_H
