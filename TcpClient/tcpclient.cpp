#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QMessageBox>
#include <QHostAddress>
#include <vcruntime.h>
#include <string.h>
#include <QTcpSocket>
#include "privatechat.h"
#include "book.h"
TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)

{
    ui->setupUi(this);
    resize(300,250);
    loadConfig();
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);
    connect(&m_tcpSocket,&QTcpSocket::connected,this,&TcpClient::ShowClient);
    connect(&m_tcpSocket,&QTcpSocket::readyRead,this,&TcpClient::recvMsg);
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{

    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray baData=file.readAll();
        QString strData=baData.toStdString().c_str();

        file.close();
        strData.replace("\r\n"," ");

        QStringList strList=strData.split(" ");

        m_strIP=strList.at(0);
        m_usPort=strList.at(1).toUShort();
        qInfo()<<"Ip"<<m_strIP<<"port"<<m_usPort;
    }
    else
    {
        QMessageBox::critical(this,"open config ","open config failed");
    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TcpClient::getLoginName()
{
    return m_strLoginName;
}

QString&TcpClient::getCurPath()
{
    return m_strCurPath;
}

void TcpClient::updateCurPath(const QString& NewCurPath)
{
    m_strCurPath=NewCurPath;
}

void TcpClient::ShowClient()
{
    QMessageBox::information(this,"连接服务器","链接服务器成功");
}

void TcpClient::recvMsg()
{

    qDebug()<<m_tcpSocket.bytesAvailable();
    qDebug()<<"**************";
    uint uiPDULen=0;//??为什么能读到大小  答:结构体第一个
    m_tcpSocket.read((char*)&uiPDULen,sizeof(uint));//读到uiPDULen
    uint uiMsgLen=uiPDULen-sizeof(PDU);       //分二次去读取
    PDU*pdu=mkPDU(uiMsgLen);
    m_tcpSocket.read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));

    switch(pdu->uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_RESPOND:
    {
        if(0==strcmp(pdu->caData,REGIST_OK))
        {
            QMessageBox::information(this,"注册","注册成功");

        }
        else if(0==strcmp(pdu->caData,REGIST_FAILED))
        {
            QMessageBox::warning(this,"注册","注册失败");
        }
        break;

    }
    case ENUM_MSG_TYPE_LOGIN_RESPOND:
    {
        if(0==strcmp(pdu->caData,LOGIN_OK))
        {
            m_strCurPath=QString("./%1").arg(m_strLoginName);
            QMessageBox::information(this,"登录","登录成功");
            OpeWidget::getInstance().show();
            this->hide();//不能关闭 关闭客户端和服务器就断开链接了


        }
        else if(0==strcmp(pdu->caData,LOGIN_FAILED))
        {
            QMessageBox::warning(this,"登录","登录失败");
        }
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
    {

        OpeWidget::getInstance().getFriend()->showAllOnluneUsr(pdu);
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
    {
        if(0==strcmp(pdu->caData,SEARCH_USR_NO))
        {
            QString name=OpeWidget::getInstance().getFriend()->m_strSearchName;
            QMessageBox::information(this,"搜索","name no exist");
        }
        else if(0==strcmp(pdu->caData,SEARCH_USR_OFFLINE))
        {
            QString name=OpeWidget::getInstance().getFriend()->m_strSearchName;
            QMessageBox::information(this,"搜索","offline");
        }
        else if(0==strcmp(pdu->caData,SEARCH_USR_ONLINE))
        {
            QString name=OpeWidget::getInstance().getFriend()->m_strSearchName;
            QMessageBox::information(this,"搜索","online");
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        //表示在线 并且不是好友
        char caName[64]={'\0'};
        char caMySelf[64]={'\0'};
        strcpy_s(caName,32,pdu->caData+32);
        int ret=QMessageBox::information(this,"添加好友",QString("%1 want to add you as friend ?").arg(caName),QMessageBox::Yes,QMessageBox::No);
        PDU*respdu=mkPDU(0);
        memcpy(respdu->caData,pdu->caData,32);
        memcpy(respdu->caData+32,pdu->caData+32,32);
        if(ret==QMessageBox::Yes)
        {
            //同意
            respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;
            m_tcpSocket.write((char*)respdu,respdu->uiPDULen);


            //QMessageBox::information(this,"添加好友","对方同意了你的好友申请");

        }
        else
        {
            //不同意
            respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
            //QMessageBox::information(this,"添加好友","对方拒绝了你的好友申请");

        }
        m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=nullptr;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
    {
        //表示 其他4中处理情况
        QMessageBox::information(this,"添加好友",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_FRIEND_AGREE_RESPOND:
    {
        QMessageBox::information(this,"好友回复",QString("对方同意了你的好友请求"));
        break;
    }
    case ENUM_MSG_TYPE_FRIEND_REFUSE_RESPOND:
    {
        QMessageBox::information(this,"好友回复",QString("对方拒绝了你的好友请求"));
        break;
    }
    case ENUM_MSG_TYPE_FLUSE_FRIEND_RESPOND:
    {
        //qInfo()<<(char*)pdu->caMsg<<(char*)pdu->caMsg+32;
        OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
    {

        //被删除好友方 客户端接收到 被删除

        char caName[32]={'\0'};
        strcpy_s(caName,32,pdu->caData);
        QMessageBox::information(this,"删除好友",QString("%1删除了你的好友").arg(caName));



        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
    {
        //发送方 客户端提示删除好友成功
        QMessageBox::information(this,"删除好友","删除好友成功");
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
    {
        //因为服务器只转发了客户端发来的没有操作
        if(PrivateChat::getInstance().isHidden())
        {

            PrivateChat::getInstance().show();

        }
        //还要在接收方这里设置一下发送给谁
        char caSendName[32]={'\0'};
        memcpy(caSendName,pdu->caData,32);//给接收方这里设置一下发送过来的名字 A通过服务器转给A的pdu给B所以A的登录名等于B中发送名
        PrivateChat::getInstance().setChatName(caSendName);

        PrivateChat::getInstance().updateMsg(pdu);


        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
    {
        OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);
        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
    {
        qInfo()<<"testtttt**";
        QMessageBox::information(this,"创建文件",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_FLUSh_FILE_RESPOND:
    {
        //刷新 进入下一级 返回上一级都一样
        OpeWidget::getInstance().getBook()->updateFileList(pdu);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_DIR_RESPOND:
    {
        QMessageBox::information(this,"删除文件夹",QString("%1").arg((char*)pdu->caMsg));

        break;

    }
    case ENUM_MSG_TYPE_DELETE_FILE_RESPOND:
    {
         QMessageBox::information(this,"删除文件",QString("%1").arg((char*)pdu->caMsg));
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
    {
        QMessageBox::information(this,"上传文件",QString("%1").arg(pdu->caData));
        qInfo()<<"test entry";
        break;
    }


    default:
        break;
    }


    free(pdu);
    pdu=nullptr;
}

#if 0
void TcpClient::on_pushButton_clicked()
{
    QString strMsg=ui->lineEdit->text();
    if(!strMsg.isEmpty())
    {
        PDU*pdu=mkPDU(strMsg.toUtf8().size()+1);
        pdu->uiMsgType=8888;

        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.toUtf8().size()+1);
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;
    }
    else
    {
        QMessageBox::warning(this,"信息发送","发送的信息不能为空");
    }
}
#endif

void TcpClient::on_login_pb_clicked()
{
    QString strName=ui->name_le->text();
    m_strLoginName=strName;
    QString strPwd=ui->pwd_le->text();
    if(!strName.isEmpty()&&!strPwd.isEmpty())
    {
        PDU*pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_LOGIN_REQUEST;
        strcpy_s(pdu->caData,32,strName.toUtf8().toStdString().c_str());
        strcpy_s(pdu->caData+32,32,strPwd.toUtf8().toStdString().c_str());
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;

    }
    else
    {
        QMessageBox::critical(this,"登录","登录失败 用户名或者密码为空:");
    }

}


void TcpClient::on_regist_pb_clicked()
{
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();
    if(!strName.isEmpty()&&!strPwd.isEmpty())
    {
        PDU*pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_REGIST_REQUEST;
        strcpy_s(pdu->caData,32,strName.toUtf8().toStdString().c_str());
        strcpy_s(pdu->caData+32,32,strPwd.toUtf8().toStdString().c_str());
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;

    }
    else
    {
        QMessageBox::critical(this,"注册","注册失败 用户名或者密码为空:");
    }

}


void TcpClient::on_cancel_pb_clicked()
{

}

