#-------------------------------------------------
#
#      Disparity and depth maps with OpenCV
#
#    by AbsurdePhoton - www.absurdephoton.fr
#
#                v1.3 - 2019/06/03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = disparity-filtering-mpo-opencv
TEMPLATE = app

LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui

SOURCES +=  main.cpp\
            mainwindow.cpp \
            mpo.cpp \
            mat-image-tools.cpp \
    disparity.cpp \
    adjust.cpp

HEADERS  += mainwindow.h \
            mpo.h \
            mat-image-tools.h \
    disparity.h \
    adjust.h

FORMS    += mainwindow.ui \
    disparity.ui \
    adjust.ui

# we add the package opencv to pkg-config
CONFIG += link_pkgconfig
PKGCONFIG += opencv4

QMAKE_CXXFLAGS += -std=c++11

# icons
RESOURCES += resources.qrc
