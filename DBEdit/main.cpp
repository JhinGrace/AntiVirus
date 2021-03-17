#include "DBEdit.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DBEdit w;
    w.show();
    return a.exec();
}
