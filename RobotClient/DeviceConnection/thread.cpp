#include "thread.h"
#include <QDebug>

InteractThread::InteractThread(QString ip, QString port, QObject *parent)
            : QThread(parent)
{
    v_socket = new VSocket(ip, port);
    run_flag = true;
}

InteractThread::~InteractThread() {
    v_socket->close_socket();
    qDebug() << "destroy the thread !";
}

void InteractThread::StopThread() {
    PackageHeader pack;
    pack.msg_type = CMD_TYPE_INSTRUCTION;
    pack.msg_data = CMD_INSTRUCTION_INTERRUPT;
    pack.msg_len = sizeof(PackageHeader);
    v_socket->sendCommand(pack);
    run_flag = false;
}

void InteractThread::SendCmd(PackageHeader header) {
    v_socket->sendCommand(header);
}

void InteractThread::SendData(char *data, int num) {
    v_socket->sendTdata(data, num);
}

void InteractThread::run() {
    dosomething();
}

void InteractThread::dosomething() {
    int ret;
    //char szbuffer[MAXBYTE];
    while (run_flag)
    {
        ret = v_socket->Receiver();
        /*
        if (!run_flag) {
            shutdown(v_socket->sock, SD_SEND);
            recv(v_socket->sock, szbuffer, MAXBYTE, NULL);
            break;
        }
        */
        if (ret == -2) { //错误
            emit SocketError();
            run_flag = false;
            qDebug() <<  "error......";
            break;
        }
        //接收成功，更新界面显示
        emit DataReceived();
    };
    qDebug() <<  "over......";
}
