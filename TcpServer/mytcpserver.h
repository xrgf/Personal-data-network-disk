#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"
class MyTcpServer : public QTcpServer

{
    Q_OBJECT
public:
    MyTcpServer();
    static MyTcpServer &getInstance();
    void incomingConnection(qintptr socketDescriptor)override;
    void resend(char*pername,PDU*pdu);
private:
    QList<MyTcpSocket*>m_tcpSocketList;
public slots:
    void deleteSocket(MyTcpSocket*mySockte);


};

#endif // MYTCPSERVER_H
