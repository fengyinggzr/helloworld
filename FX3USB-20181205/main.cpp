#include "FX3USBWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FX3USBWindow w;
    w.show();

    return a.exec();
}
