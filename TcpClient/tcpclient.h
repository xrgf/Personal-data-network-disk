#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "protocol.h"
#include "opewidget.h"
QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    static TcpClient&getInstance();
    QTcpSocket&getTcpSocket();
    QString getLoginName();
    QString&getCurPath();
    void updateCurPath(const QString& NewCurPath);
public slots:
    void ShowClient();
    void recvMsg();

private slots:
    //void on_pushButton_clicked();

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_cancel_pb_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;
    quint64 m_usPort;
    //链接服务器，和为服务器数据交互
    QTcpSocket m_tcpSocket;
    QString    m_strLoginName;
    QString   m_strCurPath;
};
#endif // TCPCLIENT_H
