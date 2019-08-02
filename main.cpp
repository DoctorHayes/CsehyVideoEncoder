#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // Values used by default for the QSettings class.
    a.setOrganizationName("Csehy Summer School of Music");
    a.setOrganizationDomain("csehy.org");
    a.setApplicationName("Csehy Video Encoder");

    w.show();

    return a.exec();
}
