#-------------------------------------------------
#
# Project created by QtCreator 2018-07-11T10:48:35
#
#-------------------------------------------------

QT       += core gui widgets winextras opengl network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RobotClient
TEMPLATE = app

INCLUDEPATH += Eigen

SOURCES += main.cpp\
        mainwindow.cpp \
    DataStructure/pointcloud.cpp \
    DeviceConnection/clientsocket.cpp \
    DeviceConnection/connection.cpp \
    DeviceConnection/thread.cpp \
    DeviceControl/devicecontrol.cpp \
    DeviceControl/dialplate.cpp \
    PointCloudViewer/opengldisplay.cpp \
    PointCloudViewer/pointcloudviewer.cpp

HEADERS  += \
    mainwindow.h \
    DataStructure/displayflag.h \
    DataStructure/nodeformat.h \
    DataStructure/packageheader.h \
    DataStructure/pointcloud.h \
    DataStructure/vpoint.h \
    DataStructure/vpose.h \
    DataStructure/vqueue.h \
    DeviceConnection/clientsocket.h \
    DeviceConnection/connection.h \
    DeviceConnection/global.h \
    DeviceConnection/socketcmd.h \
    DeviceConnection/thread.h \
    DeviceControl/devicecontrol.h \
    DeviceControl/dialplate.h \
    PointCloudViewer/opengldispay.h \
    PointCloudViewer/pointcloudview.h \
    PointCloudViewer/shader.h \
    DataStructure/correctnode.h

RESOURCES += \
    icon.qrc

DISTFILES += \
    symbol.rc

RC_FILE += \
    symbol.rc
