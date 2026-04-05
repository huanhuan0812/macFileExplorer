#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    MainWindow w;
    w.setWindowIcon(QIcon(":/icon.png"));
    w.show();

    return app.exec();
}