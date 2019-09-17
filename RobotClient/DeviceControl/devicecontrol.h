#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QToolButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include "DeviceConnection/socketcmd.h"
#include "PointCloudViewer/opengldispay.h"
#include "DataStructure/packageheader.h"
#include "dialplate.h"

typedef struct {
    bool move_forward;
    bool move_left;
    bool move_right;
    bool move_back;
    bool move_stop;
}KeyFlag;

class DeviceControl : public QWidget
{
    Q_OBJECT
public:
    DeviceControl(QWidget * parent = 0);
    ~DeviceControl();

public:
    OpenglDisplay *opengl_window_online;

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    virtual void focusInEvent(QFocusEvent *event);

signals:
    void CommandTransfer(PackageHeader);
    void MultiKeyPressed();
    void DisplayChange(DisplayFlag);

private slots:
    void ChangeControlMode();
    void MultiKeyMonitor();
    void test();
    void CheckBoxChanged();
    void OperationMode();
    void OpenPriorMap();

    void time_out_func();

private:
    void InitFunctionModule();
    void InitControlModule();
    void InitDisplayModule();
    void InitMonitorModule();
    void LayoutSet();
    void StyleConfiguration();
    void CommandProcess();

private:
    QToolButton *button_prior;

    QLabel *label_mode;
    QComboBox *combox_display_mode;
    QToolButton *button_apply;

    QLabel *label_display;
    QCheckBox *check_cloud;
    QCheckBox *check_priormap;
    QCheckBox *check_grid;
    QCheckBox *check_origin;
    QCheckBox *check_pose;
    QCheckBox *check_all;

    QLabel *label_monitor;
    QCheckBox *check_trail;

    QLabel *label_modeswitch;
    QCheckBox *checkbox_control_mode;
    QCheckBox *checkbox_send_mode;
    QTimer *timer_cmd;
    QToolButton *button_move_forward;
    QToolButton *button_move_left;
    QToolButton *button_move_right;
    QToolButton *button_move_back;
    QToolButton *button_move_stop;
    QToolButton *button_move_leftforward;
    QToolButton *button_move_rightforward;
    QToolButton *button_move_leftback;
    QToolButton *button_move_rightback;

    QHBoxLayout *layout_main;
    QVBoxLayout *layout_toolbar;
    QGridLayout *layout_button;
    QHBoxLayout *layout_mode;
    KeyFlag PressFlag;
    DisplayFlag flag_display;

private:
    bool is_continueSend;
    bool is_continueTrigger;
    int linear;
    int angular;
};

#endif // CONTROLPANEL_H
