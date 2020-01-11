#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	int ac = argc;
	QApplication a(argc, argv);
	MainWindow w(ac, argv);

	return a.exec();
}
