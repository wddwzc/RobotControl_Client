#include "mainwindow.h"

#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());    // set no task bar
//    setWindowFlags(Qt::CustomizeWindowHint);
    // set the size of mainwindow
    this->resize(1000, 700);
//    this->resize(1500, 900);
    this->setMinimumSize(500,400);
    // set the color of mainwindow
    QPalette palette = this->palette();
    //palette.setColor(QPalette::Background, QColor(0, 119, 255));
    palette.setColor(QPalette::Background, QColor(20, 20, 20));
    this->setAutoFillBackground(true);
    this->setPalette(palette);

    TitleBarInitial();      //标题栏初始化
    TabWidgetInitial();     //标签页初始化
    LayoutSet();            //布局管理器设置
    OtherWindowInitial();   //其他窗口初始化
    StyleConfiguration();   //样式表设置
    StyleChange();          //样式表局部变更
    thread_is_on = false;   //默认设备未连接
    is_maximization = false;//默认小窗口
    is_buttonpressed = false;
    p_global = PointCloudData::GetInstance();
}

Widget::~Widget() {
    if (thread_is_on) {
        emit ThreadStopping();
        thread_socket->wait();
    }
}

void Widget::TitleBarInitial() {
    button_connection = new QToolButton(this);
    button_interrupt = new QToolButton(this);
    button_extention = new QToolButton(this);
    label_status_tips = new QLabel(this);
    button_close = new QToolButton(this);
    button_maxmize = new QToolButton(this);
    button_minimize = new QToolButton(this);
    button_connection->setFixedSize(50, 50);
    button_connection->setToolTip(tr("Connect"));
    button_interrupt->setFixedSize(50, 50);
    button_interrupt->setEnabled(false);
    button_interrupt->setToolTip(tr("Disconnect"));
    button_extention->setFixedSize(50, 50);
    button_extention->setToolTip(tr("More"));
    label_status_tips->setText(tr("There is no device!"));
    connect(button_connection, &QToolButton::clicked, this, &Widget::DeviceConnection);
    connect(button_interrupt, &QToolButton::clicked, this, &Widget::DeviceInterruption);
    connect(button_close, &QToolButton::clicked, this, &Widget::TitleClicked);
    connect(button_maxmize, &QToolButton::clicked, this, &Widget::TitleClicked);
    connect(button_minimize, &QToolButton::clicked, this, &Widget::TitleClicked);
}

void Widget::TabWidgetInitial() {
    device_control = new DeviceControl(this);
    point_cloud_viewer = new PointCloudViewer(this);

    tabwidget_main_viewer = new QTabWidget(this);
    tabwidget_main_viewer->setTabPosition(QTabWidget::TabPosition::West);
    tabwidget_main_viewer->addTab(device_control, "Control");
    tabwidget_main_viewer->addTab(point_cloud_viewer, "PointCloud");
    tabwidget_main_viewer->setCurrentIndex(0);
    device_control->focusPolicy();

    connect(this, &Widget::OnlineDisplay, device_control->opengl_window_online, &OpenglDisplay::OnlineUpdate);

    //connect(tabwidget_main_viewer, &QTabWidget::currentChanged, )
}

void Widget::LayoutSet() {
    layout_title = new QHBoxLayout;
    layout_title->addWidget(button_connection);
    layout_title->addWidget(button_interrupt);
    layout_title->addWidget(button_extention);
    layout_title->addStretch();
    layout_title->addWidget(label_status_tips);
    layout_title->addStretch();
    layout_title->addWidget(button_minimize);
    layout_title->addWidget(button_maxmize);
    layout_title->addWidget(button_close);
    layout_title->setMargin(0);
    layout_main = new QVBoxLayout(this);
    layout_main->addLayout(layout_title);
    layout_main->addWidget(tabwidget_main_viewer);
    layout_main->setMargin(10);
}

void Widget::OtherWindowInitial() {
    control_panel = new Connection;
    control_panel->hide();
    connect(control_panel, &Connection::LinkInfoTransfer, this, &Widget::CreateLinks);
}

void Widget::StyleConfiguration() {
    button_close->setStyleSheet("QToolButton{ border-image: url(:/titlebar/close); } "
                                "QToolButton:hover{ border-image: url(:/titlebar/close_hover); }"
                                "QToolButton:pressed{ border-image: url(:/titlebar/close_press); }");

    button_maxmize->setStyleSheet("QToolButton{ border-image: url(:/titlebar/big); } "
                                  "QToolButton:hover{ border-image: url(:/titlebar/big_hover); }"
                                  "QToolButton:pressed{ border-image: url(:/titlebar/big_press); }");

    button_minimize->setStyleSheet("QToolButton{ border-image: url(:/titlebar/small); } "
                                   "QToolButton:hover{ border-image: url(:/titlebar/small_hover); }"
                                   "QToolButton:pressed{ border-image: url(:/titlebar/small_press); }");
    //blcak 0 20 40   blue 80 80 255

    button_extention->setStyleSheet("QToolButton{ border-image: url(:/titlebar/more); }");

    label_status_tips->setStyleSheet("QLabel{ color: white }");

    /*
    tabwidget_main_viewer->setStyleSheet("QTabWidget::pane{ border-bottom: 3px solid #EAEAEA; border-top: 3px solid #EAEAEA; background: #FAFAFA; }"
                                         "QTabWidget::tab-bar{ alignment: middle; border: none; background: rgb(20, 20, 20); }"
                                         "QTabBar::tab{ font-size: 10px; font-weight: 800; font-family: Times; border: none; color: black; "
                                         "background: white; height: 40px; min-width: 85px; padding: 5px; }"
                                         "QTabBar::tab:hover{ background: rgb(0, 0, 0); }"
                                         "QTabBar::tab:selected{ color: rgb(20, 20, 20); border-bottom: 3px solid rgb(0, 119, 255);}");
                                         */

    tabwidget_main_viewer->setStyleSheet("QTabWidget::pane{ border-bottom: 1px solid #202020; background: #303030;}"
                                         "QTabBar::tab{ border-top-left-radius: 20px; color: white; "
                                         "background: #202020; padding: 10px; height: 100px; }"
                                         "QTabBar::tab:hover{ background: #282828; }"
                                         "QTabBar::tab:selected{ background: #303030; }");
}

void Widget::StyleChange() {
    if (button_connection->isEnabled()) {
        button_connection->setStyleSheet("QToolButton{ border-image: url(:/titlebar/connect); }"
                                         "QToolButton:hover{ border-image: url(:/titlebar/connect_hover); }"
                                         "QToolButton:pressed{ border-image: url(:/titlebar/connect_press); }");
    }
    else {
        button_connection->setStyleSheet("QToolButton{ border-image: url(:/titlebar/connected); }");
    }
    if (button_interrupt->isEnabled()) {
        button_interrupt->setStyleSheet("QToolButton{ border-image: url(:/titlebar/interrupt); }"
                                        "QToolButton:hover{ border-image: url(:/titlebar/interrupt_hover); }"
                                        "QToolButton:pressed{ border-image: url(:/titlebar/interrupt_press); }");
    }
    else {
        button_interrupt->setStyleSheet("QToolButton{ border-image: url(:/titlebar/interrupt_invalid); }");
    }
}

void Widget::mousePressEvent(QMouseEvent *event) {
    window_position = event->globalPos() - pos();
    event->accept();
    //qDebug() << event->globalPos().x() << event->globalPos().y();
    //qDebug() << pos().x() << pos().y();
    //qDebug() << event->pos().x() << event->pos().y();
    //qDebug() << window_position_.x() << window_position_.y();
}

void Widget::mouseReleaseEvent(QMouseEvent *event) {
    window_position = QPoint();
    event->accept();
}

void Widget::mouseMoveEvent(QMouseEvent *event) {
    if ( event->pos().y() <= 50 && !is_maximization )
        move(event->globalPos() - window_position);
    event->accept();
}

void Widget::mouseDoubleClickEvent(QMouseEvent *event) {
    if ( event->pos().y() <= 50 )
        emit button_maxmize->clicked();
    event->accept();
}

void Widget::TitleClicked() {
    QToolButton *button_pressed = qobject_cast<QToolButton *>(sender());
    //QWidget *pWindow = this->window();
    if (this->isTopLevel()) {
        if (button_pressed == button_minimize) {
            this->showMinimized();
        }
        else if (button_pressed == button_maxmize) {
            if (this->isMaximized()) {
                this->showNormal();
                is_maximization = false;
            }
            else {
                this->showMaximized();
                is_maximization = true;
            }
            //this->isMaximized() ? this->showNormal() : this->showMaximized();
        }
        else if (button_pressed == button_close) {
            this->close();
        }
    }
}


void Widget::DeviceConnection() {
    control_panel->show();
}
// 断开通信，关闭线程
void Widget::DeviceInterruption() {
    emit ThreadStopping();
    thread_is_on = false;
    button_interrupt->setEnabled(false);
    button_connection->setEnabled(true);
    label_status_tips->setText(tr("There is no device!"));
    StyleChange();
}
// 开启线程，建立通信
void Widget::CreateLinks(QString ip, QString port) {
    qDebug() << "Linking!!!!!!";
    thread_socket = new InteractThread(ip, port);
    connect(thread_socket, &QThread::finished, thread_socket, &QObject::deleteLater);
    connect(this, &Widget::ThreadStopping, thread_socket, &InteractThread::StopThread);
    connect(device_control, &DeviceControl::CommandTransfer, thread_socket, &InteractThread::SendCmd);
    connect(point_cloud_viewer, &PointCloudViewer::DataTransfer, thread_socket, &InteractThread::SendData);
    connect(thread_socket, &InteractThread::DataReceived, this, &Widget::UpdateDisplay);
    thread_socket->start();
    thread_is_on = true;
    button_connection->setEnabled(false);
    button_interrupt->setEnabled(true);
    label_status_tips->setText(tr("Linked: ") + ip);
    StyleChange();
}

void Widget::UpdateDisplay() {
    emit OnlineDisplay();
}

