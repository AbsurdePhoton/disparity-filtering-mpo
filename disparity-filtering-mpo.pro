#-------------------------------------------------
#
#      Disparity and depth maps with OpenCV
#
#    by AbsurdePhoton - www.absurdephoton.fr
#
#                v1 - 2018/07/10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Disparity-Filtering-MPO
TEMPLATE = app

INCLUDEPATH += /usr/local/include/opencv2
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui

SOURCES +=  main.cpp\
            mainwindow.cpp \
            mpo.cpp \
            mat-image-tools.cpp

HEADERS  += mainwindow.h \
            mpo.h \
            mat-image-tools.h

FORMS    += mainwindow.ui

# we add the package opencv to pkg-config
CONFIG += link_pkgconfig
PKGCONFIG += opencv

QMAKE_CXXFLAGS += -std=c++11

# icons
RESOURCES += resources.qrc
