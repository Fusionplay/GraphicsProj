#ifndef DRAWWINDOW_H
#define DRAWWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QString>
#include <QPainter>
#include <QDebug>
#include <QPixmap>
#include <QList>
#include <QTextEdit>
#include "structs.h"
#include "drawalg.h"

namespace Ui {
class DrawWindow;
}

class DrawWindow : public QMainWindow
{
	Q_OBJECT

	enum Dr_State //drag state, 拖动的状态
	{
		still,
		topoint, //画曲线 多边形，选择操作时：待点击
		todrag
	};

	enum Prim   //当前选择绘制的图元
	{
		ellipse,
		line,
		poly,
		curve,
		trans,
		rota,
		sca,
		clip,
		unchecked
	};

	struct TP //提示点
	{
		int id;
		Point p;
		TP(int idp, Point pp) {id = idp; p = pp;}
		TP() {}
	};


public:
	explicit DrawWindow(QWidget *parent = nullptr);
	~DrawWindow();

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent* event);


private slots:
	void on_useLine_triggered(bool checked);

	void on_useEllipse_triggered(bool checked);

	void on_actSave_triggered(bool checked);

	void on_actReset_triggered(bool checked);

	void on_actRed_triggered(bool checked);

	void on_actGreen_triggered(bool checked);

	void on_actBlue_triggered(bool checked);

	void on_usePoly_triggered(bool checked);
	
	void on_useBezier_triggered();

	void on_useBspline_triggered();

	void on_useTrans_triggered();

	void on_useRotate_triggered();

	void on_useScale_triggered();

	void on_useClip_triggered();

private:
	Ui::DrawWindow *ui;

/*采用的策略：鼠标松开前，每监测到鼠标的移动，就：
 * 1. 把已有的图元的格子涂成白色
 * 2. 删除当前图元列表中的所有点
 * 3. 生成新的点
 * 4. 重新绘制已有的图元
 * 5. 绘制新的当前图元
 *
 *
 */
	int w; //width, 宽度，x轴
	int h; //height, 高度，y轴
	int unit_id;
	int curvetype; //曲线的类型：0 = Bezier 1 = Bspline
	QPixmap hehe;
	QPixmap hehedis; //用于显示到屏幕上的pixmap
	QPen pen;
	QPainter painter;
	QList<Item> item;
	Dr_State state;
	Prim prim;
	QPoint start_pos;
	QList<Point>* cur; //当前正在绘制的图元的点列表
	QList<TP> tippoint; //图元的提示点集
	QList<Point> ctrlpoint; //图元的控制点集
	int trindtip; //要平移的图元在tippoint数组中的索引
	int trind; //要平移的图元在item数组中的索引
	bool totrans;
	Point rp;  //旋转和缩放的参考点
	bool showrp; //是否显示参考点

	QString savename = "UI_output.bmp"; //"E:\\HP\\UI_output.bmp";


	//functions
	QPoint get_po(QMouseEvent *event);
	void uncheck_all_units();
	void init_vars();
	void ini_pixmap();
	void setCanvas();
	void del_units();
	void draw(QPoint point);
	void draw_to_pixmap();
	void line_clip(QPoint point);
	int getCurrentNode(QPoint p);
	bool near_rp(QPoint p);
	void iniUI(); //初始化自定义的widget
	void reset_tipp(); //旋转和缩放后，重新设置tippoint

	//自定义组件：PlainTextEdit
	QTextEdit* txtEdit;
};

#endif // DRAWWINDOW_H
