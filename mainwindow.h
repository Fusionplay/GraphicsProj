#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "display.h"
#include "drawwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(int argc, char *argv[], QWidget *parent = nullptr);
	~MainWindow();

//signals:
//	void start_file_drawing();

private slots:
//	void on_btn_File_clicked(bool checked);

	void on_btn_Mouse_clicked(bool checked);


private:
	Ui::MainWindow *ui;
	display *win;    //文件绘制窗口
	DrawWindow* drawwin;
};

#endif // MAINWINDOW_H
