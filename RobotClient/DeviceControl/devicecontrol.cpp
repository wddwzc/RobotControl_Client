#include "DeviceControl/devicecontrol.h"
#include <QDebug>

DeviceControl::DeviceControl(QWidget *parent)
    : QWidget(parent)
{
    InitControlModule();
    InitDisplayModule();
    InitMonitorModule();
    InitFunctionModule();
    StyleConfiguration();
    LayoutSet();

    is_continueSend = false;
    is_continueTrigger = false;
    linear = 0;
    angular = 0;

    this->setFocus();
}

void DeviceControl::focusInEvent(QFocusEvent *event) {
    qDebug() << "focus";
}

void DeviceControl::time_out_func(){
//    qDebug() << "test";
    PackageHeader cmd;
    cmd.msg_type = CMD_TYPE_MOVEMENT;
    cmd.msg_data = CMD_MOVE_KEEP;
    cmd.msg_len = sizeof(PackageHeader);
    emit CommandTransfer(cmd);
}

DeviceControl::~DeviceControl() {

}

void DeviceControl::keyPressEvent(QKeyEvent *event) {
    int key_pressed = event->key();
    if (key_pressed == Qt::Key_W && !event->isAutoRepeat()) {
        PressFlag.move_forward = true;
        emit MultiKeyPressed();
    }
    else if (key_pressed == Qt::Key_S && !event->isAutoRepeat()) {
        PressFlag.move_back = true;
        emit MultiKeyPressed();
    }
    else if (key_pressed == Qt::Key_A && !event->isAutoRepeat()) {
        PressFlag.move_left = true;
        emit MultiKeyPressed();
    }
    else if (key_pressed == Qt::Key_D && !event->isAutoRepeat()) {
        PressFlag.move_right = true;
        emit MultiKeyPressed();
    }
    else if (key_pressed == Qt::Key_Q && !event->isAutoRepeat()) {
        PressFlag.move_stop = true;
        emit MultiKeyPressed();
    }
}

void DeviceControl::keyReleaseEvent(QKeyEvent *event) {
    int key_released = event->key();
    if (key_released == Qt::Key_W && !event->isAutoRepeat()) {
        PressFlag.move_forward = false;
        if (is_continueTrigger) emit MultiKeyPressed();
    }
    else if (key_released == Qt::Key_S && !event->isAutoRepeat()) {
        PressFlag.move_back = false;
        if (is_continueTrigger) emit MultiKeyPressed();
    }
    else if (key_released == Qt::Key_A && !event->isAutoRepeat()) {
        PressFlag.move_left = false;
        if (is_continueTrigger) emit MultiKeyPressed();
    }
    else if (key_released == Qt::Key_D && !event->isAutoRepeat()) {
        PressFlag.move_right = false;
        if (is_continueTrigger) emit MultiKeyPressed();
    }
    else if (key_released == Qt::Key_Q && !event->isAutoRepeat()) {
        PressFlag.move_stop = false;
        if (is_continueTrigger) emit MultiKeyPressed();
    }
}

void DeviceControl::ChangeControlMode() {
    QCheckBox *checkbox_changed = qobject_cast<QCheckBox *>(sender());
    if (checkbox_changed == checkbox_send_mode) {
        if (checkbox_changed->isChecked()) {
            timer_cmd->start(60);
            is_continueSend = true;
        }
        else {
            timer_cmd->stop();
            is_continueSend = false;
        }
    }
    else if (checkbox_changed == checkbox_control_mode) {
        if (checkbox_changed->isChecked()) {
            is_continueTrigger = true;
        }
        else {
            is_continueTrigger = false;
        }
    }
}

void DeviceControl::CommandProcess() {
    int linear_add = 0;
    int angular_add = 0;
    PackageHeader cmd;
    cmd.msg_type = CMD_TYPE_MOVEMENT;
    cmd.msg_len = sizeof(PackageHeader);
    QToolButton *button_pressed = qobject_cast<QToolButton *>(sender());
    if (button_pressed == button_move_leftforward) {
        cmd.msg_data = CMD_MOVE_LEFTFORWARD;
    }
    else if (button_pressed == button_move_forward) {
        cmd.msg_data = CMD_MOVE_FORWARD;
        linear_add = 1;
    }
    else if (button_pressed == button_move_rightforward) {
        cmd.msg_data = CMD_MOVE_RIGHTFORWARD;
    }
    else if (button_pressed == button_move_left) {
        cmd.msg_data = CMD_MOVE_LEFT;
        angular_add = -1;
    }
    else if (button_pressed == button_move_stop) {
        cmd.msg_data = CMD_MOVE_STOP;
        angular = 0;
        linear = 0;
    }
    else if (button_pressed == button_move_right) {
        cmd.msg_data = CMD_MOVE_RIGHT;
        angular_add = 1;
    }
    else if (button_pressed == button_move_leftback) {
        cmd.msg_data = CMD_MOVE_LEFTBACK;
    }
    else if (button_pressed == button_move_back) {
        cmd.msg_data = CMD_MOVE_BACK;
        linear_add = -1;
    }
    else if (button_pressed == button_move_rightback) {
        cmd.msg_data = CMD_MOVE_RIGHTBACK;
    }
    //qDebug() << cmd.msg_data;

    if ((angular + angular_add) >= -4 && (angular + angular_add) <= 4) {
        angular += angular_add;
    }
    if ((linear + linear_add) >= -4 && (linear + linear_add) <= 4) {
        linear += linear_add;
    }
    opengl_window_online->UpdateStatus(linear, angular);
    emit CommandTransfer(cmd);
}

void DeviceControl::MultiKeyMonitor() {
    if (PressFlag.move_forward) {
        if (PressFlag.move_left)        emit button_move_leftforward->clicked();    //qDebug() << "AW";
        else if (PressFlag.move_right)  emit button_move_rightforward->clicked();   //qDebug() << "DW";
        else                            emit button_move_forward->clicked();        //qDebug() << "!W";
    }
    else if (PressFlag.move_back) {
        if (PressFlag.move_left)        emit button_move_leftback->clicked();       //qDebug() << "AS";
        else if (PressFlag.move_right)  emit button_move_rightback->clicked();      //qDebug() << "DS";
        else                            emit button_move_back->clicked();           //qDebug() << "!S";
    }
    else if (PressFlag.move_left)       emit button_move_left->clicked();           //qDebug() << "!A";
    else if (PressFlag.move_right)      emit button_move_right->clicked();          //qDebug() << "!D";
    else if (PressFlag.move_stop)       emit button_move_stop->clicked();           //qDebug() << "!STOP";
    /*
    else if (PressFlag.move_stop && !is_continueSend)
                                        emit button_move_stop->clicked();           //qDebug() << "!STOP";
    else if (is_modecontinuous)         emit button_move_stop->clicked();           //qDebug() << "!STOP";
    */
}

void DeviceControl::test() {
    PackageHeader cmd;
    if (check_cloud->isChecked()) {
        qDebug() << "transfer";
        cmd.msg_type = CMD_TYPE_DATA;
        cmd.msg_data = CMD_DATA_GETCLOUD;
        cmd.msg_len = sizeof(PackageHeader);
        emit CommandTransfer(cmd);
    }
    else {
        qDebug() << "stop";
        cmd.msg_type = CMD_TYPE_DATA;
        cmd.msg_data = CMD_DATA_ENDCLOUD;
        cmd.msg_len = sizeof(PackageHeader);
        emit CommandTransfer(cmd);
    }
}

void DeviceControl::CheckBoxChanged() {
    QCheckBox *cbx_statechanged = qobject_cast<QCheckBox *>(sender());
    if (cbx_statechanged == check_all) {
        if (check_all->isChecked()) {
            check_cloud->setChecked(true);
            check_priormap->setChecked(true);
            check_grid->setChecked(true);
            check_origin->setChecked(true);
            check_pose->setChecked(true);
        }
        else {
            check_cloud->setChecked(false);
            check_priormap->setChecked(false);
            check_grid->setChecked(false);
            check_origin->setChecked(false);
            check_pose->setChecked(false);
        }
    }
    else {
        if (check_cloud->isChecked() && check_priormap->isChecked() && check_grid->isChecked() && check_origin->isChecked() && check_pose->isChecked()) {
            check_all->setChecked(true);
        }
        if (!check_cloud->isChecked() || !check_priormap->isChecked() || !check_grid->isChecked() || !check_origin->isChecked() || !check_pose->isChecked()) {
            check_all->setChecked(false);
        }
    }
    if (check_cloud->isChecked())   flag_display.cloud = true;
    else                            flag_display.cloud = false;
    if (check_priormap->isChecked())flag_display.prior = true;
    else                            flag_display.prior = false;
    if (check_grid->isChecked())    flag_display.grid = true;
    else                            flag_display.grid = false;
    if (check_origin->isChecked())  flag_display.origin = true;
    else                            flag_display.origin = false;
    if (check_pose->isChecked())    flag_display.pose = true;
    else                            flag_display.pose = false;
    if (check_trail->isChecked())   flag_display.trail = true;
    else                            flag_display.trail = false;
    PackageHeader cmd;
    if (cbx_statechanged == check_cloud && flag_display.cloud) {
        cmd.msg_type = CMD_TYPE_DATA;
        cmd.msg_data = CMD_DATA_GETCLOUD;
        cmd.msg_len = 0;
    }
    else if (cbx_statechanged == check_cloud && !flag_display.cloud) {
        cmd.msg_type = CMD_TYPE_DATA;
        cmd.msg_data = CMD_DATA_ENDCLOUD;
        cmd.msg_len = 0;
    }
    if (cbx_statechanged == check_pose && flag_display.pose) {
        cmd.msg_type = CMD_TYPE_DATA;
        cmd.msg_data = CMD_DATA_GETPOSE;
        cmd.msg_len = 0;
    }
    else if (cbx_statechanged == check_pose && !flag_display.pose) {
        cmd.msg_type = CMD_TYPE_DATA;
        cmd.msg_data = CMD_DATA_ENDPOSE;
        cmd.msg_len = 0;
    }
    emit DisplayChange(flag_display);
    emit CommandTransfer(cmd);
}

void DeviceControl::OperationMode() {
    PackageHeader cmd;
    cmd.msg_type = CMD_TYPE_INSTRUCTION;
    int index = combox_display_mode->currentIndex();
    qDebug() << combox_display_mode->currentText();
    if (index == 0) {
        cmd.msg_data = CMD_INSTRUCTION_MANUAL;
    }
    else if (index == 1) {
        cmd.msg_data = CMD_INSTRUCTION_NAVIGATION;
    }
    else if (index == 2) {
        cmd.msg_data = CMD_INSTRUCTION_MAPPING;
    }
    cmd.msg_len = 0;
    emit CommandTransfer(cmd);
}

void DeviceControl::OpenPriorMap() {
    opengl_window_online->OpenFile();
}

void DeviceControl::InitFunctionModule() {
    button_prior = new QToolButton(this);
    button_prior->setText(tr("Load Prior Map"));
    connect(button_prior, &QToolButton::clicked, this, &DeviceControl::OpenPriorMap);

    label_mode = new QLabel(this);
    label_mode->setText(tr("Mode Selection:"));
    combox_display_mode = new QComboBox(this);
    combox_display_mode->addItem(tr("Manual"));
    combox_display_mode->addItem(tr("Navigation"));
    combox_display_mode->addItem(tr("Mapping"));
    button_apply = new QToolButton(this);
    button_apply->setText(tr("Apply"));
    connect(button_apply, &QToolButton::clicked, this, &DeviceControl::OperationMode);

}

void DeviceControl::InitControlModule() {
    timer_cmd = new QTimer(this);
    label_modeswitch = new QLabel(tr("Continuous"), this);
    checkbox_control_mode = new QCheckBox(tr("Trigger"), this);
    checkbox_send_mode = new QCheckBox(tr("Send"));
    connect(timer_cmd, &QTimer::timeout, this, &DeviceControl::time_out_func);
    connect(checkbox_control_mode, &QCheckBox::stateChanged, this, &DeviceControl::ChangeControlMode);
    connect(checkbox_send_mode, &QCheckBox::stateChanged, this, &DeviceControl::ChangeControlMode);
    connect(this, &DeviceControl::MultiKeyPressed, this, &DeviceControl::MultiKeyMonitor);

    checkbox_send_mode->setChecked(true);

    button_move_leftforward = new QToolButton(this);
    button_move_forward = new QToolButton(this);
    button_move_rightforward = new QToolButton(this);
    button_move_left = new QToolButton(this);
    button_move_stop = new QToolButton(this);
    button_move_right = new QToolButton(this);
    button_move_leftback = new QToolButton(this);
    button_move_back = new QToolButton(this);
    button_move_rightback = new QToolButton(this);
    button_move_leftforward->setFixedSize(60, 60);
    button_move_forward->setFixedSize(60, 60);
    button_move_rightforward->setFixedSize(60, 60);
    button_move_left->setFixedSize(60, 60);
    button_move_stop->setFixedSize(60, 60);
    button_move_right->setFixedSize(60, 60);
    button_move_leftback->setFixedSize(60, 60);
    button_move_back->setFixedSize(60, 60);
    button_move_rightback->setFixedSize(60, 60);
    connect(button_move_leftforward, &QToolButton::clicked, this, &DeviceControl::CommandProcess);
    connect(button_move_forward, &QToolButton::clicked, this, &DeviceControl::CommandProcess);
    connect(button_move_rightforward, &QToolButton::clicked, this, &DeviceControl::CommandProcess);
    connect(button_move_left, &QToolButton::clicked, this, &DeviceControl::CommandProcess);
    connect(button_move_stop, &QToolButton::clicked, this, &DeviceControl::CommandProcess);
    connect(button_move_right, &QToolButton::clicked, this, &DeviceControl::CommandProcess);
    connect(button_move_leftback, &QToolButton::clicked, this, &DeviceControl::CommandProcess);
    connect(button_move_back, &QToolButton::clicked, this, &DeviceControl::CommandProcess);
    connect(button_move_rightback, &QToolButton::clicked, this, &DeviceControl::CommandProcess);
    PressFlag.move_back = false;
    PressFlag.move_forward = false;
    PressFlag.move_left = false;
    PressFlag.move_right = false;
}

void DeviceControl::InitDisplayModule() {
    opengl_window_online = new OpenglDisplay(true, this);

    label_display = new QLabel(this);
    label_display->setText(tr("Sensor & Display:"));
    check_cloud = new QCheckBox(tr("Cloud"), this);
    check_priormap = new QCheckBox(tr("Prior Map"), this);
    check_grid = new QCheckBox(tr("Grid"), this);
    check_origin = new QCheckBox(tr("Origin"), this);
    check_pose = new QCheckBox(tr("Pose"), this);
    check_all = new QCheckBox(tr("All"), this);
    check_priormap->setChecked(true);
    check_grid->setChecked(true);
    check_origin->setChecked(true);
    connect(check_cloud, &QCheckBox::clicked, this, &DeviceControl::CheckBoxChanged);
    connect(check_priormap, &QCheckBox::clicked, this, &DeviceControl::CheckBoxChanged);
    connect(check_grid, &QCheckBox::clicked, this, &DeviceControl::CheckBoxChanged);
    connect(check_origin, &QCheckBox::clicked, this, &DeviceControl::CheckBoxChanged);
    connect(check_pose, &QCheckBox::clicked, this, &DeviceControl::CheckBoxChanged);
    connect(check_all, &QCheckBox::clicked, this, &DeviceControl::CheckBoxChanged);
    connect(this, &DeviceControl::DisplayChange, opengl_window_online, &OpenglDisplay::DisplayFlagChange);
    flag_display.cloud = false;
    flag_display.grid = false;
    flag_display.origin = false;
    flag_display.path = false;
    flag_display.picked = false;
    flag_display.pose = false;
    flag_display.prior = false;
    flag_display.trail = false;
}

void DeviceControl::InitMonitorModule() {
    label_monitor = new QLabel(this);
    label_monitor->setText(tr("Monitoring Settings:"));
    check_trail = new QCheckBox(this);
    check_trail->setText(tr("Position Trailing"));
    connect(check_trail, &QCheckBox::clicked, this, &DeviceControl::CheckBoxChanged);
}

void DeviceControl::LayoutSet() {
    layout_button = new QGridLayout;
    layout_button->addWidget(label_modeswitch, 0, 0, 1, 3);
    layout_mode = new QHBoxLayout;
    layout_mode->addWidget(checkbox_send_mode);
    layout_mode->addWidget(checkbox_control_mode);
    layout_button->addLayout(layout_mode, 1, 0, 1, 3);
    layout_button->addWidget(button_move_leftforward, 2, 0, 1, 1);
    layout_button->addWidget(button_move_forward, 2, 1, 1, 1);
    layout_button->addWidget(button_move_rightforward, 2, 2, 1, 1);
    layout_button->addWidget(button_move_left, 3, 0, 1, 1);
    layout_button->addWidget(button_move_stop, 3, 1, 1, 1);
    layout_button->addWidget(button_move_right, 3, 2, 1, 1);
    layout_button->addWidget(button_move_leftback, 4, 0, 1, 1);
    layout_button->addWidget(button_move_back, 4, 1, 1, 1);
    layout_button->addWidget(button_move_rightback, 4, 2, 1, 1);
    layout_button->setMargin(0);
    layout_toolbar = new QVBoxLayout;
    layout_toolbar->addWidget(button_prior);
    //layout_toolbar->addStretch();
    layout_toolbar->addSpacing(10);
    layout_toolbar->addWidget(label_mode);
    layout_toolbar->addWidget(combox_display_mode);
    layout_toolbar->addWidget(button_apply, 0, Qt::AlignRight);
    //layout_toolbar->addStretch();
    layout_toolbar->addSpacing(10);
    layout_toolbar->addWidget(label_display);
    layout_toolbar->addWidget(check_cloud);
    layout_toolbar->addWidget(check_priormap);
    layout_toolbar->addWidget(check_grid);
    layout_toolbar->addWidget(check_origin);
    layout_toolbar->addWidget(check_pose);
    layout_toolbar->addWidget(check_all);
    //layout_toolbar->addStretch();
    layout_toolbar->addSpacing(10);
    layout_toolbar->addWidget(label_monitor);
    layout_toolbar->addWidget(check_trail);
    layout_toolbar->addStretch();
    layout_toolbar->addLayout(layout_button);
    layout_main = new QHBoxLayout(this);
    layout_main->addLayout(layout_toolbar);
    layout_main->addWidget(opengl_window_online);
}

void DeviceControl::StyleConfiguration() {
    const char button_stylesheet[] = "QToolButton{ background: #CCCCCC; width: 200px; padding: 5px; border-radius: 10px; }"
                                     "QToolButton:hover{ background: #FFFFFF; }"
                                     "QToolButton:pressed{ background: #999999; }";
    const char checkbox_stylesheet[] = "QCheckBox{ color: white; width: 70px; }";
    const char label_stylesheet[] = "QLabel{ color: white; }";


    label_modeswitch->setStyleSheet(label_stylesheet);
    checkbox_send_mode->setStyleSheet(checkbox_stylesheet);
    checkbox_control_mode->setStyleSheet(checkbox_stylesheet);
    button_move_leftforward->setStyleSheet("QToolButton{ border-image: url(:/wheel/lf); }"
                                           "QToolButton:hover{ border-image: url(:/wheel/lf_hover); }"
                                           "QToolButton:pressed{ border-image: url(:/wheel/lf_press); }");

    button_move_forward->setStyleSheet("QToolButton{ border-image: url(:/wheel/forward); }"
                                       "QToolButton:hover{ border-image: url(:/wheel/forward_hover); }"
                                       "QToolButton:pressed{ border-image: url(:/wheel/forward_press); }");

    button_move_rightforward->setStyleSheet("QToolButton{ border-image: url(:/wheel/rf); }"
                                            "QToolButton:hover{ border-image: url(:/wheel/rf_hover); }"
                                            "QToolButton:pressed{ border-image: url(:/wheel/rf_press); }");

    button_move_left->setStyleSheet("QToolButton{ border-image: url(:/wheel/left); }"
                                    "QToolButton:hover{ border-image: url(:/wheel/left_hover); }"
                                    "QToolButton:pressed{ border-image: url(:/wheel/left_press); }");

    button_move_stop->setStyleSheet("QToolButton{ border-image: url(:/wheel/stop); }"
                                    "QToolButton:hover{ border-image: url(:/wheel/stop_hover); }"
                                    "QToolButton:pressed{ border-image: url(:/wheel/stop_press); }");

    button_move_right->setStyleSheet("QToolButton{ border-image: url(:/wheel/right); }"
                                     "QToolButton:hover{ border-image: url(:/wheel/right_hover); }"
                                     "QToolButton:pressed{ border-image: url(:/wheel/right_press); }");

    button_move_leftback->setStyleSheet("QToolButton{ border-image: url(:/wheel/lb); }"
                                        "QToolButton:hover{ border-image: url(:/wheel/lb_hover); }"
                                        "QToolButton:pressed{ border-image: url(:/wheel/lb_press); }");

    button_move_back->setStyleSheet("QToolButton{ border-image: url(:/wheel/back); }"
                                    "QToolButton:hover{ border-image: url(:/wheel/back_hover); }"
                                    "QToolButton:pressed{ border-image: url(:/wheel/back_press); }");

    button_move_rightback->setStyleSheet("QToolButton{ border-image: url(:/wheel/rb); }"
                                         "QToolButton:hover{ border-image: url(:/wheel/rb_hover); }"
                                         "QToolButton:pressed{ border-image: url(:/wheel/rb_press); }");

    check_cloud->setStyleSheet(checkbox_stylesheet);
    check_priormap->setStyleSheet(checkbox_stylesheet);
    check_grid->setStyleSheet(checkbox_stylesheet);
    check_origin->setStyleSheet(checkbox_stylesheet);
    check_pose->setStyleSheet(checkbox_stylesheet);
    check_all->setStyleSheet(checkbox_stylesheet);
    checkbox_control_mode->setStyleSheet(checkbox_stylesheet);
    check_trail->setStyleSheet(checkbox_stylesheet);
    label_mode->setStyleSheet(label_stylesheet);
    label_display->setStyleSheet(label_stylesheet);
    label_monitor->setStyleSheet(label_stylesheet);
    button_prior->setStyleSheet(button_stylesheet);
}


