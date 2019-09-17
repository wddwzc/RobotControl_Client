#include "clientsocket.h"
#include <QDebug>

VSocket::VSocket(QString ip, QString port)
            : link_ip(ip), link_port(port)
{
    pData = PointCloudData::GetInstance();
    buf = new char[BUF_SIZE];

    QByteArray temp_array = link_ip.toLocal8Bit();
    c_link_ip = temp_array.data();
    uint_link_port = link_port.toUInt();
    qDebug() << link_ip << "--" << link_port << "char:" << c_link_ip;
    is_connected = false;
    if (InitSocket()) {
        qDebug() << "You can send data now...";
    }
}

VSocket::~VSocket() {
    delete[]buf;
    if(sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
        WSACleanup();
    }
}

bool VSocket::InitSocket() {
    int err;
    WSADATA wsaData;    // 这结构是用于接收Wjndows Socket的结构信息的
    err = WSAStartup(MAKEWORD(1, 1), &wsaData); // 初始化1.1版本的WinSocket库
    if( err != 0 )
        return false;          // 返回值为零的时候是表示成功申请WSAStartup
    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
        // 检查版本信息是否符合要求，否则，调用WSACleanup()清除信息，结束函数
           WSACleanup();
           return false;
    }
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0){
        qDebug() << "socket establish failed...";
    }
    if(0 == connectServer()) {
        is_connected = true;
        return true;
    }
    is_connected = false;
    return false;
}

void VSocket::sendCommand(PackageHeader cmd) {
    if(false == is_connected)
        return;
    char send_buf[64];
    char *ch = send_buf;
    memcpy(ch, (char*)(&cmd), sizeof(PackageHeader));
    int len = sizeof(PackageHeader);
    cmd.msg_len = len;
    //ch = ch + sizeof(CmdType);
    int ret = send(sock, send_buf, len, NULL);
    if (ret <= 0) {
        qDebug() << "failed to send command message";
        return;
    }
    qDebug() << "[" << QString(QLatin1String(send_buf)) << "]";
}

void VSocket::sendTdata(char *data, int num) {
    if(false == is_connected)
        return;
    int ret = send(sock, data, num, NULL);
    if (ret <= 0) {
        qDebug() << "failed to send data";
        return;
    }
}

void VSocket::close_socket() {
    /*
    PackageHeader pack;
    pack.msg_type = CMD_TYPE_INSTRUCTION;
    pack.msg_data = CMD_INSTRUCTION_INTERRUPT;
    pack.msg_len = sizeof(PackageHeader);
    sendTdata(pack);
    Receiver();
    */
    if(sock != INVALID_SOCKET){
        qDebug() << "destroy the socket!";
        closesocket(sock);
        sock = INVALID_SOCKET;
        WSACleanup();
    }
}

int VSocket::connectServer() {
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(uint_link_port);
    address.sin_addr.s_addr = inet_addr(c_link_ip);
    if(address.sin_addr.s_addr == INADDR_NONE)
        return -1;
    int connected = connect(sock, (SOCKADDR *)&address, sizeof(SOCKADDR));
    if(0 == connected)
        qDebug() << "connnected! ";
    return connected;
}

int VSocket::Receiver() {
    if(false == is_connected)
        return -2;
    qDebug() << "waiting: ";
    int ret = RecvData(sizeof(PackageHeader), buf);
    if(ret == -1) //正常超时
        return -1;
    else if(ret == 0 || ret == -2) //异常
        return -2;
    PackageHeader *readbuf = (PackageHeader *)buf;
    int m_datanum = readbuf->msg_len;
    qDebug() << "Receiving: " << readbuf->msg_len;
    CmdType type = readbuf->msg_type;
    CmdType cmd_data = readbuf->msg_data;
    switch (type) {
        case CMD_TYPE_DATA: {
            if (cmd_data == CMD_DATA_GETCLOUD) {
                const int len = m_datanum * sizeof(VPointI);
                ret = RecvData(len, buf);
                if(ret == -1) //正常超时
                    return -1;
                else if(ret == 0 || ret == -2) //异常
                    return -2;
                //解析，并存储至全局队列pData->point_que
                qDebug() << "Analyse pointcloud data !";
                pData->que_lock.lock();
                char *pp = buf;
                VPointI* pmsg;
                for (int i = 0; i < m_datanum; i++) {
                    pmsg = (VPointI*)pp;
                    pp += sizeof(VPointI);
                    pData->point_que.push(*pmsg);   //存至全局队列
                }
                pData->que_lock.unlock();           //解锁
                qDebug() << "receive cloud";
            }
            else if (cmd_data == CMD_DATA_GETPOSE) {
                const int len = m_datanum;
                ret = RecvData(len, buf);
                if(ret == -1) //正常超时
                    return -1;
                else if(ret == 0 || ret == -2) //异常
                    return -2;
                //解析，并存储至全局队列pData->point_que
                qDebug() << "Analyse pointcloud data !";
                pData->que_lock.lock();
                VPoseStamped *pmsg = (VPoseStamped*)buf;
                pData->RobotPose = *pmsg;
                pData->que_lock.unlock();           //解锁
                qDebug() << "receive pose";
            }
            break;
        }
        case CMD_TYPE_MOVEMENT: {
            qDebug() << "MOVEMENT";
            break;
        }
        case CMD_TYPE_INSTRUCTION: {
            if (readbuf->msg_data == CMD_INSTRUCTION_INTERRUPT) {
                qDebug() << "interrup";
            }
            break;
        }
        default: {
            qDebug() << "not joined";
            break;
        }
    }
    return 0;
}

int VSocket::RecvData(int len, char *buffer) {
    int count = 0;
    int cnt = 0;
    while(count < len) {
        int ret = recv(sock, buffer, len - count, NULL);
        if ((ret < 0) && (errno != EINTR) && (errno != EWOULDBLOCK) && (errno != EAGAIN) && (errno != 0)) { //出错了
            std::cout << "connect failure! errno:"<<errno<< endl;
            return -2;
        }
        else if((ret < 0) && (errno == EINTR))  //被中断，继续
        {
            continue;
        }
        else if((ret < 0) && ((errno == EWOULDBLOCK) || (errno == EAGAIN) || (errno == 0)))  //接收超时，先返回（-1）
        {
            if(count)//接收到一半超时了，则出错了，接不上了！
            {
                if(cnt>40) {
                    qDebug() << "time out";
                    return -2;
                }
                cnt++;
                continue;
            }
            return -1; //没接收到数据就超时了，正常，先返回
        }
        else if(ret == 0)  //socket被关闭
        {
            std::cout << "close socekt by the other side!"<< endl;
            return -2;
        }
        count += ret;
        buffer += ret;
        qDebug() << "received: " << count << " / " << len << " : " << ret;
    }
    return count;
}
