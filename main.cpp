/*#-------------------------------------------------
#
#      Disparity and depth maps with OpenCV
#
#    by AbsurdePhoton - www.absurdephoton.fr
#
#                v1 - 2018/07/10
#
#-------------------------------------------------*/

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
