#ifndef STRUCTS_H
#define STRUCTS_H

#include <QPen>
#include <QList>

struct Point
{
	int x;
	int y;
	Point(int xp, int yp) { x = xp; y = yp;}
	Point() { x = y = 0; }
};


struct Item
{
	enum Type
	{
		ELLI, //椭圆
		LINE, //直线
		POLYGON, //多边形
		CURVE //曲线
	};

	//图元的裁剪，旋转等操作需要的变量，存储图元中的特殊点和信息。
	Point sp; //直线的起点
	Point ep; //直线的终点
	Point mid; //椭圆的中心点
	int a; //椭圆的x轴半径
	int b; //椭圆的y轴半径
	int curvetype; //曲线的类型：0 = bezier 1 = B-spline
	QList<Point> *nodelist = nullptr; //多边形或曲线的节点数组

	//图元的其它信息
	Type type; //图元的类型
	int id;    //图元ID
	QPen pen;  //画笔信息，用来存储绘制当前图元的画笔的颜色
	QList<Point> *plist; //图元的点集
	Item(int idpa, QList<Point> *ppa, const QPen& p, Type t) {id = idpa; plist = ppa; pen = p; type = t;}
//	Item(QList<Point> *ppa) {plist = ppa;}
};

#endif // STRUCTS_H
