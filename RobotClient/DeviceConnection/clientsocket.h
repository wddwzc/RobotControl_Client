#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <QMessageBox>
#include <QString>

#pragma comment(lib,"ws2_32.lib")

#include "DataStructure/packageheader.h"
#include "DataStructure/vpoint.h"
#include "DataStructure/vpose.h"
#include "DataStructure/pointcloud.h"

#define BUF_SIZE 2000000

class VSocket
{
public:
    VSocket(QString ip, QString port);
    ~VSocket();
    SOCKET sock;

public:
    bool InitSocket();
    void sendCommand(PackageHeader cmd);
    void sendTdata(char *data, int num);
    void close_socket();
    int Receiver();

private:
    int connectServer();

    int RecvData(int len, char* buffer);

    char *buf;
    QString link_ip;
    QString link_port;
    char *c_link_ip;
    unsigned int uint_link_port;
    bool is_connected;
    PointCloudData *pData;
};

#endif // CLIENTSOCKET_H
