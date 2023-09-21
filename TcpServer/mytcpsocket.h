#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include <QFile>
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString&getm_strName();
    void flushList(PDU*pdu);
signals:
    void offline(MyTcpSocket*mysocket);


public slots:
    void recvMsg();
    void clientOffiline();
private:
    QString m_strName;

    QFile m_file;
    qint64 m_iTotal;
    qint64 m_iRecved;
    bool m_bUpload;

};

#endif // MYTCPSOCKET_H
