#ifndef CONNECT_H
#define CONNECT_H

#include <QWidget>
#include <QToolButton>
#include <QTableView>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QProcess>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

class Connection : public QWidget
{
    Q_OBJECT

public:
    Connection(QWidget *parent = 0);
    ~Connection();
    void ObjectInitial();
    void LayoutSet();

signals:
    void LinkInfoTransfer(QString, QString);

private slots:
    void StartLink();
    void ExitDialog();
    void UpdateIPList();
    bool PingTest(QString ip_test);
    void ChoosingIP();

private:
    QToolButton *button_confirm;
    QToolButton *button_cancel;
    QToolButton *button_updateList;
    QLabel *local_hostname;
    QLabel *local_ip;
    QLineEdit *ip_edit;
    QLineEdit *port_edit;
    QTableView *iplist_viewer;
    QVBoxLayout *layout_main;
    QVBoxLayout *layout_button;
    QHBoxLayout *layout_viewer;
    QHBoxLayout *layout_edit;
    QHBoxLayout *layout_label;
    QStandardItemModel *item_model;
};

#endif // CONNECT_H
