#include "robit.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    robit w;
    w.setWindowTitle("RO:GIT");
    w.setWindowIcon(QIcon(":/image/resources/logo.jpeg"));
    w.setFixedSize(1320, 720);
    w.show();

    return a.exec();
}
