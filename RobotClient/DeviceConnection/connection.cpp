#include "connection.h"

Connection::Connection(QWidget *parent)
        : QWidget(parent)
{
    //this->hide();
    this->setFixedSize(500, 300);
    ObjectInitial();
    LayoutSet();
}

Connection::~Connection() {

}
//控件初始化
void Connection::ObjectInitial() {
    local_hostname = new QLabel(this);
    local_ip = new QLabel(this);
    ip_edit = new QLineEdit(this);
    port_edit = new QLineEdit(this);
    button_confirm = new QToolButton(this);
    button_cancel = new QToolButton(this);
    button_updateList = new QToolButton(this);
    iplist_viewer = new QTableView(this);
    item_model = new QStandardItemModel(this);

    connect(button_confirm, &QToolButton::clicked, this, &Connection::StartLink);
    connect(button_cancel, &QToolButton::clicked, this, &Connection::ExitDialog);
    connect(button_updateList, &QToolButton::clicked, this, &Connection::UpdateIPList);
    connect(iplist_viewer, &QTableView::doubleClicked, this, &Connection::ChoosingIP);

    local_hostname->setText(tr("----"));
    local_ip->setText(tr("----"));
    ip_edit->setText(tr("0.0.0.0"));
    port_edit->setText(tr("12533"));
    button_confirm->setText(tr("  OK  "));
    button_cancel->setText(tr("Cancel"));
    button_updateList->setText(tr("Update"));


    item_model->setColumnCount(1);
    item_model->setHeaderData(0, Qt::Horizontal, tr("IP Address of Device"));
    iplist_viewer->setSelectionBehavior(QTableView::SelectRows);   //行选中
    iplist_viewer->setAlternatingRowColors(true);   //交替使用背景色
    iplist_viewer->setEditTriggers(QTableView::NoEditTriggers);     //禁止编辑
    iplist_viewer->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    iplist_viewer->resizeColumnsToContents();
    //iplist_viewer->setFocusPolicy(Qt::NoFocus);   //选中无虚框
    iplist_viewer->setModel(item_model);

}
//布局管理器设置
void Connection::LayoutSet() {
    layout_label = new QHBoxLayout;
    layout_label->addWidget(local_ip);
    layout_label->addWidget(local_hostname);
    layout_edit = new QHBoxLayout;
    layout_edit->addWidget(ip_edit);
    layout_edit->addWidget(port_edit);
    layout_button = new QVBoxLayout;
    layout_button->addWidget(button_confirm);
    layout_button->addWidget(button_cancel);
    layout_button->addWidget(button_updateList);
    layout_button->addStretch();
    layout_viewer = new QHBoxLayout;
    layout_viewer->addWidget(iplist_viewer);
    layout_viewer->addLayout(layout_button);
    layout_main = new QVBoxLayout(this);
    layout_main->addLayout(layout_label);
    layout_main->addLayout(layout_edit);
    layout_main->addLayout(layout_viewer);
}
//网络连接IP及端口信息设置
void Connection::StartLink() {
    QString ip_link =  ip_edit->text();
    QString port_link = port_edit->text();
    if (PingTest(ip_link)) {
        emit LinkInfoTransfer(ip_link, port_link);
        this->hide();
        return;
    }
    else {
        QMessageBox::warning(this, tr("Error"), tr("The address is invalid!"), QMessageBox::Ok);
    }

}
//退出对话框
void Connection::ExitDialog() {
    this->hide();
}
//更新链接列表
void Connection::UpdateIPList() {
    QString hostname = QHostInfo::localHostName();          //获取主机名
    QString network_address;                                //IP地址网络段
    QHostInfo info = QHostInfo::fromName(hostname);         //主机信息
    local_hostname->setText(hostname);
    foreach (QHostAddress address, info.addresses()) {      //遍历IP地址
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            if (!address.toString().startsWith("169")) {
                local_ip->setText(address.toString());
                network_address = address.toString();
                break;
            }
        }
    }
    network_address = network_address.left(network_address.lastIndexOf(".") + 1);
    ip_edit->setText(network_address);

    item_model->setItem(0, 0, new QStandardItem(tr("192.168.1.102")));

//    int useRow = 0;
//    for (int i = 0; i < 10; ++i) {
//        qDebug() << i;
//        QHostInfo host = QHostInfo::fromName(network_address + QString("%1").arg(i));
//        if(host.error() != QHostInfo::NoError){

//            qDebug() << "Lookup failed:" << host.errorString();
//            continue;
//        }

//        if(host.addresses()[0].toString() == host.hostName())
//            continue;

//        QStandardItem *ipItem = new QStandardItem;
//        ipItem->setText(host.addresses()[0].toString());
//        QStandardItem *hostItem = new QStandardItem;
//        hostItem->setText(host.hostName());

////        if(ipItem->text() != hostItem->text()){

////            ipItem->setTextColor(QColor(Qt::red));
////            hostItem->setTextColor(QColor(Qt::red));
////        }
//        item_model->setItem(useRow, 0, ipItem);
//        item_model->setItem(useRow, 1, hostItem);
//        useRow++;
//    }
}
//通过ping测试IP地址有效性
bool Connection::PingTest(QString ip_test) {
    QProcess *cmd = new QProcess;
    QString str_cmd = "ping " + ip_test + " -n 1 -w " + QString::number(2.0);
    cmd->start(str_cmd);
    cmd->waitForReadyRead();
    cmd->waitForFinished();
    QString str_return = cmd->readAll();
    if (str_return.indexOf("TTL") != -1) {
        return true;
    }
    return false;
}

void Connection::ChoosingIP() {
    QModelIndex index = iplist_viewer->currentIndex();
    if (index.isValid()) {
        ip_edit->setText(item_model->itemData(index).value(0).toString());
    }
}
