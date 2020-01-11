#include <limits>
#include "drawalg.h"

const double PI = 3.1416;
typedef unsigned long long ull;

void line_DDA(QList<Point> *line, int x1, int y1, int x2, int y2)
{
	int temp;
	if (x1 > x2)
	{
		temp = x2;
		x2 = x1;
		x1 = temp;
		temp = y2;
		y2 = y1;
		y1 = temp;
	}

	if (x1 == x2)
	{
		if (y1 > y2)
		{
			int temp = y2;
			y2 = y1;
			y1 = temp;
		}

		for (int i = y1; i <= y2; i++)
			line->append(Point(x1, i));
	}

	else if (y1 == y2)
	{
		for (int i = x1; i <= x2; i++)
			line->append(Point(i, y1));
	}

	else
	{
		int len;
		if ((x2 - x1) >= abs(y2 - y1))
			len = x2 - x1;
		else
			len = abs(y2 - y1);

		double delta_x = (x2 - x1) / (len * 1.0);
		double delta_y = (y2 - y1) / (len * 1.0);
		double xtemp = x1 + 0.5;
		double ytemp = y1 + 0.5;

		for (int i=0;i<len;i++)
		{
			line->append(Point((int)xtemp, (int)ytemp));
			xtemp += delta_x;
			ytemp += delta_y;
		}
		line->append(Point(x2, y2));
	}
}


void line_Bre(QList<Point> *line, int x1, int y1, int x2, int y2)
{
	int x, y, dx, dy, s1, s2, p, toswap, i;
	x = x1;
	y = y1;
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);

	if (x2 > x1)
		s1 = 1;
	else
		s1 = -1;
	if (y2 > y1)
		s2 = 1;
	else
		s2 = -1;

	if (dy > dx)
	{
		int temp = dx;
		dx = dy;
		dy = temp;
		toswap = 1;
	}
	else
		toswap = 0;

	p = 2 * dy - dx;
	for (i = 1; i <= dx; i++)
	{
		line->append(Point(x, y));
		if (p >= 0)
		{
			if (toswap == 0)
				y += s2;
			else
				x += s1;
			p = p - 2 * dx;
		}
		if (toswap == 0)
			x += s1;
		else
			y += s2;

		p += 2 * dy;
	}
	line->append(Point(x2, y2));
}


inline void add4Points(QList<Point> *line, int x0, int y0, int x, int y)
{
	line->append(Point(x0 + x, y0 + y));
	line->append(Point(x0 + x, y0 - y));
	line->append(Point(x0 - x, y0 + y));
	line->append(Point(x0 - x, y0 - y));
}


void ellipse_mid(QList<Point> *elli, int x0, int y0, int a, int b)
{
	bool is_laying = true;

	if (b > a)
	{
		is_laying = false;
		int temp = b;
		b = a;
		a = temp;
	}

	double sqa = a * a;
	double sqb = b * b;

	double d = sqb + sqa * (0.25 - b);
	int x = 0;
	int y = b;

	//首个添加的
	if (is_laying)
		add4Points(elli, x0, y0, x, y); //x=0, y=b
	else
		add4Points(elli, x0, y0, y, x); //x=b, y=0
	//椭圆的第一象限的上半部分
	while (sqb*(x + 1) < sqa*(y - 0.5))
	{
		if (d < 0)
			d += sqb*(2 * x + 3);

		else
		{
			d += (sqb*(2 * x + 3) + sqa*((-2)*y + 2));
			y--;
		}

		x++;
		if (is_laying)
			add4Points(elli, x0, y0, x, y);
		else
			add4Points(elli, x0, y0, y, x);
	}

	//椭圆的第一象限的下半部分
	d = (b * (x + 0.5)) * 2 + (a * (y - 1)) * 2 - (a * b) * 2;
	while (y >= 0)
	{
		if (d < 0)
		{
			d += sqb * (2 * x + 2) + sqa * ((-2) * y + 3);
			x++;
		}

		else
			d += sqa * ((-2) * y + 3);
		y--;
		if (is_laying)
			add4Points(elli, x0, y0, x, y);
		else
			add4Points(elli, x0, y0, y, x);
	}
}


inline void trans_point(Point &po, int dx, int dy)
{
	po.x += dx;
	po.y += dy;
}


void translate(Item* it, int dx, int dy)
{
	for (Point &po: *it->plist)
		trans_point(po, dx, dy);

	switch (it->type)
	{
	case Item::LINE:
		trans_point(it->sp, dx, dy);
		trans_point(it->ep, dx, dy);
		break;
	case Item::ELLI:
		trans_point(it->mid, dx, dy);
		break;
	case Item::CURVE:;
	case Item::POLYGON:
		for (Point &po: *it->nodelist)
			trans_point(po, dx, dy);
		break;

	}
}


void translate(QList<Point> *plist, int dx, int dy)
{
	for (Point &po: *plist)
	{
		po.x += dx;
		po.y += dy;
	}
}


inline void rotate_point(Point &po, int x, int y, double theta)
{
	int xtmp = po.x - x;
	int ytmp = po.y - y;
	po.x = (int)(xtmp * cos(theta) - ytmp * sin(theta) + x);
	po.y = (int)(xtmp * sin(theta) + ytmp * cos(theta) + y);
}


void rotate(Item* it, int x, int y, int r)
{
	//如果是椭圆，则旋转全部点
	//若是线段则旋转两个端点，多边形则旋转几个节点
	double theta = (PI / 180) * (double)r;

	if (it->type == Item::ELLI) //椭圆：旋转中心点，然后重新生成
	{
//		for (Point &po: *it->plist)
//			rotate_point(po, x, y, theta);
		rotate_point(it->mid, x, y, theta);
		it->plist->clear();
		ellipse_mid(it->plist, it->mid.x, it->mid.y, it->a, it->b);
	}

	else if (it->type == Item::LINE)
	{
		it->plist->clear(); //清空点集
		Point sp = it->sp;
		Point ep = it->ep;
		rotate_point(sp, x, y, theta);
		rotate_point(ep, x, y, theta);
		line_DDA(it->plist, sp.x, sp.y, ep.x, ep.y); //使用DDA算法重新生成线段
		it->sp = sp;
		it->ep = ep;
	}

	else if (it->type == Item::POLYGON) //多边形
	{
		it->plist->clear(); //清空点集
		int len = it->nodelist->size();
		for (int i = 0; i < len; ++i) //旋转多边形的各个节点
			rotate_point((*it->nodelist)[i], x, y, theta);

		for (int i = 0; i < len; ++i) //重新生成各条直线
		{
			line_DDA(it->plist, (*it->nodelist)[i].x, (*it->nodelist)[i].y, \
					 (*it->nodelist)[(i+1)%len].x, (*it->nodelist)[(i+1)%len].y);
		}
	}

	else if (it->type == Item::CURVE) //曲线
	{
		it->plist->clear(); //清空点集
		int len = it->nodelist->size();
		for (int i = 0; i < len; ++i) //旋转曲线的各个节点
			rotate_point((*it->nodelist)[i], x, y, theta);

		//重新生成曲线
		if (it->curvetype == 0) //bezier
			Bezier(it->plist, len, it->nodelist);
		else
			Bspline(it->plist, len, it->nodelist);
	}
}


inline void scale_point(Point &po, double s, double xtmp, double ytmp)
{
	po.x = (int)(po.x * s + xtmp);
	po.y = (int)(po.y * s + ytmp);
}


void scale(Item* it, int x, int y, double s)
{
	//对线段，多边形，椭圆：都是缩放重要端点，然后重新绘制

	double xtmp = x * (1 - s);
	double ytmp = y * (1 - s);
	it->plist->clear(); //清空点集

	if (it->type == Item::ELLI)
	{
		it->a = (int)(it->a * s);
		it->b = (int)(it->b * s);
		scale_point(it->mid, s, xtmp, ytmp);
		ellipse_mid(it->plist, it->mid.x, it->mid.y, it->a, it->b);
	}

	else if (it->type == Item::LINE)
	{
		scale_point(it->sp, s, xtmp, ytmp);
		scale_point(it->ep, s, xtmp, ytmp);
		line_DDA(it->plist, it->sp.x, it->sp.y, it->ep.x, it->ep.y); //使用DDA算法重新生成线段
	}

	else if (it->type == Item::POLYGON)
	{
		int len = it->nodelist->size();
		for (int i = 0; i < len; ++i) //旋转多边形的各个节点
			scale_point((*it->nodelist)[i], s, xtmp, ytmp);

		for (int i = 0; i < len; ++i) //重新生成各条直线
		{
			line_DDA(it->plist, (*it->nodelist)[i].x, (*it->nodelist)[i].y, \
					 (*it->nodelist)[(i+1)%len].x, (*it->nodelist)[(i+1)%len].y);
		}
	}

	else if (it->type == Item::CURVE) //曲线
	{
		it->plist->clear(); //清空点集
		int len = it->nodelist->size();
		for (int i = 0; i < len; ++i) //旋转曲线的各个节点
			scale_point((*it->nodelist)[i], s, xtmp, ytmp);

		//重新生成曲线
		if (it->curvetype == 0) //bezier
			Bezier(it->plist, len, it->nodelist);
		else
			Bspline(it->plist, len, it->nodelist);
	}
}


inline int encode(Point p, int xmin, int xmax, int ymin, int ymax)
{
	int code = 0;
	if (p.x < xmin)      //左
		code += 1;
	else if (p.x > xmax) //右
		code += 2;
	if (p.y < ymin)      //下
		code += 4;
	else if (p.y > ymax) //上
		code += 8;
	return code;
}


bool clip_cohen(Item *it, int x1, int y1, int x2, int y2)
{
	/*
	 *首先找出线段的首点和终点，encode之。

	*/

	Point sp = it->sp;
	Point ep = it->ep;
	int xmin = x1;
	int xmax = x2;
	int ymin = y1;
	int ymax = y2;
	int codesp = encode(sp, xmin, xmax, ymin, ymax);
	int codeep = encode(ep, xmin, xmax, ymin, ymax);

	double slope;
	double slopemax = std::numeric_limits<double>::max();
	if (sp.y == ep.y)
		slope = 0;
	else if (sp.x == ep.x)
		slope = slopemax;
	else
		slope = ((ep.y - sp.y) * 1.0) / (ep.x - sp.x);

	while (codesp != 0 || codeep != 0) //直到两者都等于0才结束
	{
		if ((codesp & codeep) != 0) //完全在窗口外
			return true; //要删除该item

		if ((codesp | codeep) == 0) //完全在窗口内
			break;

		//处理大于上界的情况
		if ((codesp & 8) != 0) //sp处于上方
		{
			if (slope != slopemax)
				sp.x -= (sp.y - ymax) / slope;
			sp.y = ymax;
			codesp = encode(sp, xmin, xmax, ymin, ymax);
		}
		else if ((codeep & 8) != 0) //ep处于上方
		{
			if (slope != slopemax)
				ep.x -= (ep.y - ymax) / slope;
			ep.y = ymax;
			codeep = encode(ep, xmin, xmax, ymin, ymax);
		}

		//处理小于下界的情况
		else if ((codesp & 4) != 0) //sp处于下方
		{
			if (slope != slopemax)
				sp.x -= (sp.y - ymin) / slope;
			sp.y = ymin;
			codesp = encode(sp, xmin, xmax, ymin, ymax);
		}
		else if ((codeep & 4) != 0) //ep处于下方
		{
			if (slope != slopemax)
				ep.x -= (ep.y - ymin) / slope;
			ep.y = ymin;
			codeep = encode(ep, xmin, xmax, ymin, ymax);
		}

		//处理大于右界的点
		else if ((codesp & 2) != 0)
		{
			sp.y -= slope * (sp.x - xmax);
			sp.x = xmax;
			codesp = encode(sp, xmin, xmax, ymin, ymax);
		}
		else if ((codeep & 2) != 0)
		{
			ep.y -= slope * (ep.x - xmax);
			ep.x = xmax;
			codeep = encode(ep, xmin, xmax, ymin, ymax);
		}

		//处理小于左边界的点
		else if ((codesp & 1) != 0)
		{
			sp.y -= slope * (sp.x - xmin);
			sp.x = xmin;
			codesp = encode(sp, xmin, xmax, ymin, ymax);
		}
		else if ((codeep & 1) != 0)
		{
			ep.y -= slope * (ep.x - xmin);
			ep.x = xmin;
			codeep = encode(ep, xmin, xmax, ymin, ymax);
		}
	}

	//重新绘制直线
	it->plist->clear();
	line_DDA(it->plist, sp.x, sp.y, ep.x, ep.y);
	it->sp = sp;
	it->ep = ep;
	return false;
}


bool ClipT(int p, int q, float &u1, float &u2)
{        //-dx,   p1.x - x1,    u1,      u2
	float r;
	if (p < 0)
	{
		r = (float)q / p;
		if (r > u2)
			return false;
		if (r > u1)
			u1 = r;
	}
	else if (p > 0)
	{
		r = (float)q / p;
		if (r < u1)
			return false;
		if (r < u2)
			u2 = r;
	}
	else
		return q >= 0;
	return true;
}
bool clip_liang(Item *it, int x1, int y1, int x2, int y2)
{
	float u1 = 0;
	float u2 = 1;
	Point p1 = it->sp;
	Point p2 = it->ep;
	bool t = false;
	int dx = p2.x - p1.x;
	int dy = p2.y - p1.y;
	if (ClipT(-dx, p1.x - x1, u1, u2))
	{
		if (ClipT(dx, x2 - p1.x, u1, u2))
		{
			if (ClipT(-dy, p1.y - y1, u1, u2))
			{
				if (ClipT(dy, y2 - p1.y, u1, u2))
					t = true;
			}
		}
	}

	if (t) //重新绘制直线
	{
		it->plist->clear();
		it->sp = Point(p1.x + u1 * dx, p1.y + u1 * dy);
		it->ep = Point(p1.x + u2 * dx, p1.y + u2 * dy);
		line_DDA(it->plist, it->sp.x, it->sp.y, it->ep.x, it->ep.y);
		return false;
	}

	else
		return true;
}


ull fac(int n)
{
	ull res = 1;
	for (int i = 1; i <= n; ++i)
		res *= i;
	return res;
}


ull combination(int m, int n)
{
	return fac(m) / (fac(n) * fac(m - n));
}


double bez(int i, int n, double u)
{
	return combination(n, i) * pow(u, i) * pow(1 - u, n - i);
}


void Bezier(QList<Point> *plist, int pnum, QList<Point> *dots)
{
	int dotnum = 128 * pnum;
	double interval = 1.0 / dotnum;
	double u = 0.0;
	Point tmp;
	double t;
	double xt;
	double yt;

	for (int i = 0; i <= dotnum; ++i)
	{
		xt = yt = 0.0;
		for (int j = 0; j < pnum; ++j)
		{
			tmp = (*dots)[j];
			t = bez(j, pnum-1, u);
			xt += (double)tmp.x * t;
			yt += (double)tmp.y * t;
		}

		plist->append(Point((int)(xt + 0.5), (int)(yt + 0.5)));
		u += interval;
	}
}


double N1(int i, double u) //1次
{
	double t = u - i;

	if (0 <= t && t < 1)
		return t;
	if (1 <= t && t < 2)
		return 2 - t;
	return 0;
}

double N2(int i, double u)
{
	double t = u - i;

	if (0 <= t && t < 1)
		return 0.5 * t * t;
	if (1 <= t && t < 2)
		return 3*t - t*t - 1.5;
	if (2 <= t && t < 3)
		return 0.5 * pow(3 - t, 2);
	return 0;
}

double N3(int i, double u)
{
	double t = u - i;
	double a = 1.0 / 6.0;

	if (0 <= t && t < 1)
		return a*t*t*t;
	if (1 <= t && t < 2)
		return a * (-3*pow(t-1, 3) + 3*pow(t-1, 2) + 3*(t-1) + 1);
	if (2 <= t && t < 3)
		return a * (3*pow(t-2, 3) - 6*pow(t-2, 2) + 4);
	if (3 <= t && t < 4)
		return a * pow(4-t, 3);
	return 0;
}

double N(int k, int i, double u)
{
	switch(k)
	{
	case 1:
		return N1(i, u);
	case 2:
		return N2(i, u);
	case 3:
		return N3(i, u);
	}
}


void Bspline(QList<Point> *plist, int pnum, QList<Point> *dots)
{
	int currentK = 1;
	Point tmp;
	double t;
	double xt;
	double yt;
	double intval = 1.0 / 256.0;

	for (double u = currentK; u < pnum; u += intval)
	{
		xt = yt = 0.0;
		for (int i = 0; i < pnum; ++i)
		{
			tmp = (*dots)[i];
			t = N(currentK, i, u);
			xt += (double)tmp.x * t;
			yt += (double)tmp.y * t;
		}
		plist->append(Point((int)(xt + 0.5), (int)(yt + 0.5)));
	}
}
