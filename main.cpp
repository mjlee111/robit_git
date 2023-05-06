#include "robit.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    robit w;
    w.show();

    return a.exec();
}
