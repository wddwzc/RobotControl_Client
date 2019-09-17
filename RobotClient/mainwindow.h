#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPoint>
#include <QMouseEvent>
#include <QLabel>

#include "DeviceConnection/connection.h"
#include "DeviceConnection/thread.h"
#include "DeviceControl/devicecontrol.h"
#include "PointCloudViewer/pointcloudview.h"
#include "DataStructure/pointcloud.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

public:
    void TitleBarInitial();
    void TabWidgetInitial();
    void LayoutSet();
    void OtherWindowInitial();
    void StyleConfiguration();
    void StyleChange();

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    //void wheelEvent(QWheelEvent *event);

signals:
    void ThreadStopping();
    void OnlineDisplay();

private slots:
    void TitleClicked();
    void DeviceConnection();
    void DeviceInterruption();
    void CreateLinks(QString ip, QString port);
    void UpdateDisplay();

private:
    QToolButton *button_close;
    QToolButton *button_maxmize;
    QToolButton *button_minimize;
    QToolButton *button_connection;
    QToolButton *button_interrupt;
    QToolButton *button_extention;
    QLabel *label_status_tips;
    QTabWidget *tabwidget_main_viewer;

    Connection *control_panel;
    PointCloudViewer *point_cloud_viewer;
    DeviceControl *device_control;
    QPoint mouse_position;
    QPoint window_position;
    QVBoxLayout *layout_main;
    QHBoxLayout *layout_state;
    QHBoxLayout *layout_title;
    InteractThread *thread_socket;

    bool thread_is_on;
    bool is_maximization;
    bool is_buttonpressed;

    PointCloudData *p_global;
};

#endif // WIDGET_H
