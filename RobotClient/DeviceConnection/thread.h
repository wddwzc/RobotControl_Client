#ifndef THREAD_H
#define THREAD_H

#include "clientsocket.h"
#include <QThread>
#include <QString>

class InteractThread : public QThread
{
    Q_OBJECT
public:
    InteractThread(QString ip, QString port, QObject *parent = 0);
    ~InteractThread();

public slots:
    void StopThread();
    void SendCmd(PackageHeader header);
    void SendData(char *data, int num);

protected:
    virtual void run();

signals:
    void SocketError();
    void DataReceived();

private:
    void dosomething();

    VSocket *v_socket;
    bool run_flag;
};

#endif // THREAD_H
