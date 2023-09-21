#include "mytcpserver.h"
#include <QDebug>
MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qInfo()<<"new Client connected";
    MyTcpSocket*pTcpSocket=new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);
    connect(pTcpSocket,&MyTcpSocket::offline,this,&MyTcpServer::deleteSocket);
}

void MyTcpServer::resend(char *pername, PDU *pdu)
{
    if(nullptr==pername||nullptr==pdu)
    {
        return ;
    }
    //获得客户端
    QString strName=pername;

    for(int i=0;i<m_tcpSocketList.size();i++)
    {
        if(strName==m_tcpSocketList.at(i)->getm_strName())
        {
            m_tcpSocketList.at(i)->write((char*)pdu,pdu->uiPDULen);
            break;
        }
    }
}

void MyTcpServer::deleteSocket(MyTcpSocket *mySockte)
{
    auto iter=m_tcpSocketList.begin();
    for(;iter!=m_tcpSocketList.end();iter++)
    {
        if(mySockte==*iter)
        {
            (*iter)->deleteLater();//释放list里面的值
            *iter=nullptr;
            m_tcpSocketList.erase(iter);//释放list中的占位
            break;
        }
    }
    //qDebug()<<"测试这里的输出";
    for(int i=0;i<m_tcpSocketList.size();i++)
    {
        qDebug()<<m_tcpSocketList.at(i)->getm_strName();
    }

}
