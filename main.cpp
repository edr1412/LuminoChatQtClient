#include "widget.h"
#include "common.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget *w = new Widget;
    w->setAttribute(Qt::WA_DeleteOnClose);
    return a.exec();
}
