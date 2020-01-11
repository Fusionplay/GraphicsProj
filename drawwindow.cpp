#include <stdlib.h>
#include <algorithm>
#include "drawwindow.h"
#include "ui_drawwindow.h"

const int WIDTH = 1000;
const int HEIGHT = 700;
const int offset = 47;

DrawWindow::DrawWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::DrawWindow),
	w(WIDTH),
	h(HEIGHT),
	hehe(w, h),
	painter(&hehe)
{
	ui->setupUi(this);
	ui->centralwidget->setMouseTracking(true);
	init_vars();
	iniUI();
	pen.setWidth(1);
	pen.setColor(Qt::red);
	ini_pixmap();
	ui->actRed->setChecked(true);
}


DrawWindow::~DrawWindow()
{
	delete ui;
}


void DrawWindow::iniUI()
{
	ui->toolBar->addSeparator();
	txtEdit = new QTextEdit;
	txtEdit->setMinimumWidth(10);
	txtEdit->setMaximumHeight(28);
	txtEdit->setText("1");
//	txtEdit->s
	ui->toolBar->addWidget(txtEdit);
}


void DrawWindow::reset_tipp()
{
	switch (item[trind].type)
	{
	case Item::LINE:
		tippoint[trindtip].p = item[trind].sp;
		break;
	case Item::ELLI:
		tippoint[trindtip].p = item[trind].mid;
		break;
	case Item::CURVE: ;
	case Item::POLYGON:
		tippoint[trindtip].p = (*item[trind].plist)[0];
		break;
	}
}


void DrawWindow::uncheck_all_units() //将所有按钮设为false
{
	ui->useEllipse->setChecked(false);
	ui->useLine->setChecked(false);
	ui->usePoly->setChecked(false);
	ui->useBezier->setChecked(false);
	ui->useBspline->setChecked(false);

	ui->useTrans->setChecked(false);
	ui->useRotate->setChecked(false);
	ui->useScale->setChecked(false);

	ui->useClip->setChecked(false);

	showrp = false;
}


void DrawWindow::init_vars()
{
	state = still;
	prim = unchecked;
	cur = nullptr;
	unit_id = 0;
	totrans = false;
	rp = Point(20, 20);
//	factor = 1;
	uncheck_all_units();
}


void DrawWindow::ini_pixmap()
{
	painter.setBrush(QBrush(Qt::white));
	painter.drawRect(-1, -1, w + 1, h + 1);
}


void DrawWindow::del_units()
{
	for (Item &it: item)
	{
		delete it.plist;
		if (it.nodelist != nullptr)
			delete it.nodelist;
	}
	item.clear();

	ctrlpoint.clear();
	tippoint.clear();
}


QPoint DrawWindow::get_po(QMouseEvent *event)
{
	QPoint p = event->pos();
	p.ry() -= offset;
	return p;
}


void DrawWindow::paintEvent(QPaintEvent *event)
{
	hehedis = hehe;
	//画tip point
	QPainter p(&hehedis);
	QPen cPen(QColor(0,0,255));
	cPen.setWidth(6);
	p.setPen(cPen);
	for (int i = 0; i < tippoint.size(); ++i)
		p.drawPoint(QPoint(tippoint[i].p.x, tippoint[i].p.y));

	//画提示字段
	cPen.setColor(QColor(10,200,50));
	cPen.setWidth(4);
	p.setPen(cPen);
	p.setFont(QFont("Consolas", 8));
	for (int i = 0; i < tippoint.size(); ++i)
		p.drawText(QPoint(tippoint[i].p.x + 5, tippoint[i].p.y), QString("P%1").arg(tippoint[i].id));

	//画rp及其提示字段
	if (showrp)
	{
		cPen.setColor(QColor(0,0,255));
		cPen.setWidth(6);
		p.setPen(cPen);
		p.drawPoint(QPoint(rp.x, rp.y));

		cPen.setColor(QColor(255,0,50));
		cPen.setWidth(4);
		p.setPen(cPen);
		p.setFont(QFont("Consolas", 10));
		p.drawText(QPoint(rp.x + 5, rp.y), QString("C"));
	}

	QPalette palette = ui->dr_canvas->palette();
	palette.setBrush(QPalette::Window, QBrush(hehedis));
	ui->dr_canvas->setPalette(palette);
}


void DrawWindow::setCanvas()
{
	QPalette palette = ui->dr_canvas->palette();
	palette.setBrush(QPalette::Window, QBrush(hehe));
	ui->dr_canvas->setPalette(palette);
}


int DrawWindow::getCurrentNode(QPoint p)
{
	//qDebug("getCurrentNode invoked!");
	for (int i = 0; i < tippoint.size(); ++i)
	{
		double dx = tippoint[i].p.x - p.x();
		double dy = tippoint[i].p.y - p.y();
		double length = sqrt(dx * dx + dy * dy);
		//qDebug("dis = %lf!", length);
		if (length < 10)
		{
			//qDebug("dis < 10!");
			trindtip = i;
			int trid = tippoint[i].id;
			for (int j = 0; j < item.size(); ++j)
			{
				if (item[j].id == trid)
				{
					trind = j;
					break;
				}
			}
			return i;
		}
	}
	return -1;
}


bool DrawWindow::near_rp(QPoint p)
{
	double dx = rp.x - p.x();
	double dy = rp.y - p.y();
	return sqrt(dx * dx + dy * dy) < 10;
}


void DrawWindow::draw(QPoint point)
{
	/*
   1. 把已有的图元的格子涂成白色
   2. 删除当前图元列表中的所有点
   3. 生成新的点
   4. 重新绘制已有的图元
   5. 绘制新的当前图元

   */
	//qDebug("OK here beg!");
	if (prim == unchecked)
		return;

	ini_pixmap(); //清空画布
	if (cur != nullptr)
		cur->clear(); //清空当前列表中all点

	//生成新的点
	int s;
	switch (prim)
	{
	case line:
		line_DDA(cur, start_pos.x(), start_pos.y(), point.x(), point.y());
		break;

	case ellipse:
		//ellipse_mid(QList<Point>* elli, int x0, int y0, int a, int b);
		int x0, y0, a, b;
		x0 = (point.x() + start_pos.x()) / 2;
		y0 = (point.y() + start_pos.y()) / 2;
		a = abs(point.x() - start_pos.x()) / 2;
		b = abs(point.y() - start_pos.y()) / 2;

		ellipse_mid(cur, x0, y0, a, b);
		break;

	case poly:
		s = ctrlpoint.size();
		if (s > 1)
		{
			for (int i = 0; i < s; i++)
				line_Bre(cur, ctrlpoint[i].x, ctrlpoint[i].y, ctrlpoint[(i+1)%s].x, ctrlpoint[(i+1)%s].y);
		}
		break;

	case curve:
		s = ctrlpoint.size();
		if (curvetype == 0) //Bezier
			Bezier(cur, s, &ctrlpoint);
		else
			Bspline(cur, s, &ctrlpoint);
		break;

	default: break;
	}
	//画已有的图元
	draw_to_pixmap();

	//画当前的图元
	if (cur != nullptr && cur->size() > 0)
	{
		painter.setPen(pen);
		for (Point &p: *cur)
			painter.drawPoint(p.x, p.y);
	}
}


void DrawWindow::draw_to_pixmap()
{
	for (Item &it: item)
	{
		painter.setPen(it.pen);
		for (Point &po: *it.plist)
			painter.drawPoint(po.x, po.y);
	}
}


void DrawWindow::mousePressEvent(QMouseEvent *event)
{
	/*鼠标左键按下事件
	 * 处理方式：
	 * 1. 将状态设置为待拖动
	 * 2. 获取当前的位置并存起来
	 *
	*/

	if (event->button() == Qt::LeftButton)
	{
		start_pos = get_po(event); //event->pos(); //获得窗口的坐标

		if (state == still) //刚刚开始画图: 添加控制点！
		{
			if (prim == line || prim == ellipse || prim == clip)
				state = todrag;
			else if (prim == trans)
			{
				if (totrans)
					state = todrag;
			}
			else if (prim == rota || prim == sca)
			{
				if (totrans)
				{
					if (near_rp(start_pos))//(getCurrentNode(start_pos) < 0)
						state = todrag;

					else //找到要旋转的点了
					{
						if (prim == rota)
							rotate(&item[trind], rp.x, rp.y, txtEdit->toPlainText().toInt());
						else //sca
							scale(&item[trind], rp.x, rp.y, txtEdit->toPlainText().toDouble());
						reset_tipp();
						draw(QPoint());
					}
				}
			}
			else
				state = topoint;

			//添加提示点，产生新数组：只有画图元时才要产生新的cur和提示点！
			if (prim != rota && prim != sca && prim != clip && prim != trans)
			{
				cur = new QList<Point>;
				if (prim == line || prim == poly)
					tippoint.append(TP(unit_id, Point(start_pos.x(), start_pos.y())));
			}

			if (state == topoint) //添加控制点
				ctrlpoint.append(Point(start_pos.x(), start_pos.y()));
		}

		else if (state == topoint) //需要增加控制点
		{
			switch (prim)
			{
			case poly:
				ctrlpoint.append(Point(start_pos.x(), start_pos.y())); //添加控制点
				//绘制新的图元
				draw(QPoint());
				break;

			case curve:
				ctrlpoint.append(Point(start_pos.x(), start_pos.y())); //添加控制点
				draw(QPoint());
				break;
			default: break;
			}
		}
	}

	else if (event->button() == Qt::RightButton) //按下右键：结束多边形，曲线的绘制
	{
		if (state == topoint && (prim == poly || prim == curve))
		{
			state = still;
			Item tmp(unit_id, cur, pen, (Item::Type)prim);
			tmp.nodelist = new QList<Point>;
			*(tmp.nodelist) = ctrlpoint;

			if (prim == curve) //设置曲线类型，添加曲线的tip点s
			{
				tmp.curvetype = curvetype;
				tippoint.append(TP(unit_id, (*cur)[0]));
			}

			item.append(tmp);
			++unit_id;
			ctrlpoint.clear();
			cur = nullptr;
		}
	}

	update();
	QMainWindow::mousePressEvent(event); //否则让父类来处理
}


void DrawWindow::mouseMoveEvent(QMouseEvent *event)
{
	/*鼠标拖动事件
	 * 处理方式：
	 * 1. 获取当前的位置并存起来
	 * 2. 将当前位置传给draw(),由该函数负责根据当前位置和开始位置来画Pixmap
	 * 3. 将画好的pixmap显示在canvas上
	 *
	*/
	QPoint point = get_po(event);
	//qDebug("mouseMoveEvent: state = %d, prim = %d!", (int)state, (int)prim);
	if (state == todrag)
	{
		if (prim != clip && prim != trans && prim != rota && prim != sca)
		{
			draw(point);
			update();
		}
	}

	else if (prim == trans) //平移
	{
		if (state == still)
		{
			if (getCurrentNode(point) >= 0) //不是鼠标左键是按住的状态：没按键的状态，同时移动到了某个点上
			{
				//qDebug("here!");
				setCursor(QCursor(Qt::SizeAllCursor));
				totrans = true;
			}

			else  //其它情况下：普通的鼠标图案
			{
				setCursor(QCursor(Qt::ArrowCursor));
				totrans = false;
			}

		}
	}

	else if (showrp) //旋转和缩放状态
	{
		if (state == still)
		{
			if (near_rp(point))
			{
				setCursor(QCursor(Qt::SizeAllCursor));
				totrans = true;
			}
			else if (getCurrentNode(point) >= 0)
			{
				setCursor(QCursor(Qt::PointingHandCursor));
				totrans = true;
			}
			else
			{
				setCursor(QCursor(Qt::ArrowCursor));
				totrans = false;
			}
		}
	}

	QMainWindow::mouseMoveEvent(event); //否则让父类来处理
}


void DrawWindow::mouseReleaseEvent(QMouseEvent *event)
{
	/*鼠标释放事件
	 * 处理方式：
	 * 1. 将状态设置为等待
	 * 2. 将当前的图元添加到图元列表中
	 *
	*/
	QPoint point = get_po(event); //->pos();
	if (state == todrag)
	{
		state = still;
		//Item(int idpa, QList<Point> *ppa, const QPen& p, Type t)

		if (prim == clip)
		{
			line_clip(point);
			draw(QPoint());
		}

		else if (prim == trans)
		{
			int dx = point.x() - start_pos.x();
			int dy = point.y() - start_pos.y();
			translate(&item[trind], dx, dy);
			draw(QPoint());
			tippoint[trindtip].p.x += dx;
			tippoint[trindtip].p.y += dy;
		}

		else if (showrp) //旋转或缩放
		{
			rp.x = point.x();
			rp.y = point.y();
		}

		else //结束直线，椭圆的绘制
		{
			Item tmp(unit_id, cur, pen, (Item::Type)prim);
			if (prim == line)
			{
				//start_pos.x(), start_pos.y() - offset, point.x(), point.y() - offset
				tmp.sp = Point(start_pos.x(), start_pos.y());
				tmp.ep = Point(point.x(), point.y());
			}
			else if (prim == ellipse) //还需设置tip点
			{
				int x0 = (point.x() + start_pos.x()) / 2;
				int y0 = (point.y() + start_pos.y()) / 2;
				int a = abs(point.x() - start_pos.x()) / 2;
				int b = abs(point.y() - start_pos.y()) / 2;
				tmp.mid = Point(x0, y0);
				tmp.a = a;
				tmp.b = b;
				tippoint.append(TP(unit_id, tmp.mid));
			}

			item.append(tmp);
			++unit_id;
			cur = nullptr;
		}
		update();
	}


	QMainWindow::mouseReleaseEvent(event); //否则让父类来处理
}


void DrawWindow::line_clip(QPoint point)
{
	int x1 = start_pos.x() > point.x() ? point.x() : start_pos.x();
	int x2 = start_pos.x() > point.x() ? start_pos.x() : point.x();
	int y1 = start_pos.y() > point.y() ? point.y() : start_pos.y();
	int y2 = start_pos.y() > point.y() ? start_pos.y() : point.y();
	int itid;
	bool todel;
	//qDebug("sp = %d, %d, ep = %d, %d", x1, y1, x2, y2);

	for (int i = 0; i < item.size(); ++i)
	{
		if (item[i].type == Item::LINE)
		{
			todel = false;
			itid = item[i].id;
			todel = clip_cohen(&item[i], x1, y1, x2, y2);
			//todel = clip_liang(&item[i], x1, y1, x2, y2);
			if (todel)
			{
				for (int j = 0; j < tippoint.size(); ++j)
				{
					if (tippoint[j].id == itid)
					{
						tippoint.removeAt(j);
						break;
					}
				}
				item.removeAt(i); //这里后面应改为用专门的函数实现图元的删除，因为图元的删除还涉及tippoint的删除
				--i;
			}

			else //无需删除直线，但需要重新设置tip点
			{
				for (int j = 0; j < tippoint.size(); ++j)
				{
					if (tippoint[j].id == itid)
					{
						tippoint[j].p = item[i].sp;
						break;
					}
				}
			}
		}
	}
}


void DrawWindow::on_actRed_triggered(bool checked)
{
	ui->actRed->setChecked(true);
	ui->actGreen->setChecked(false);
	ui->actBlue->setChecked(false);
	pen.setColor(Qt::red);
}

void DrawWindow::on_actGreen_triggered(bool checked)
{
	ui->actRed->setChecked(false);
	ui->actGreen->setChecked(true);
	ui->actBlue->setChecked(false);
	pen.setColor(Qt::green);
}

void DrawWindow::on_actBlue_triggered(bool checked)
{
	ui->actRed->setChecked(false);
	ui->actGreen->setChecked(false);
	ui->actBlue->setChecked(true);
	pen.setColor(Qt::blue);
}


void DrawWindow::on_useLine_triggered(bool checked)
{
	uncheck_all_units();
	ui->useLine->setChecked(true);
	prim = line;
}


void DrawWindow::on_useEllipse_triggered(bool checked)
{
	uncheck_all_units();
	ui->useEllipse->setChecked(true);
	prim = ellipse;
}


void DrawWindow::on_actSave_triggered(bool checked)
{
	hehe.save(savename);
}

void DrawWindow::on_actReset_triggered(bool checked)
{
	/*reset画布。要做的事：
	 * 1. 清空画布：画布变白
	 * 2. 清空画布中的图元列表
	 * 3. 设置各个参数值至初始状态

	*/
	ini_pixmap();
	del_units();
	init_vars();
	update();
}


void DrawWindow::on_usePoly_triggered(bool checked)
{
	uncheck_all_units();
	ui->usePoly->setChecked(true);
	prim = poly;
}

void DrawWindow::on_useBezier_triggered()
{
	uncheck_all_units();
	ui->useBezier->setChecked(true);
	prim = curve;
	curvetype = 0;
}

void DrawWindow::on_useBspline_triggered()
{
	uncheck_all_units();
	ui->useBspline->setChecked(true);
	prim = curve;
	curvetype = 1;
}

void DrawWindow::on_useTrans_triggered()
{
	uncheck_all_units();
	ui->useTrans->setChecked(true);
	prim = trans;
}

void DrawWindow::on_useRotate_triggered()
{
	uncheck_all_units();
	ui->useRotate->setChecked(true);
	prim = rota;
	showrp = true;
	update();
}

void DrawWindow::on_useScale_triggered()
{
	uncheck_all_units();
	ui->useScale->setChecked(true);
	prim = sca;
	showrp = true;
	update();
}

void DrawWindow::on_useClip_triggered()
{
	uncheck_all_units();
	ui->useClip->setChecked(true);
	prim = clip;
}


