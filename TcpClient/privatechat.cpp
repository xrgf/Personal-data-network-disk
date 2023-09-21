#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QMessageBox>
PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{

    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString strName)
{
    m_strChatName=strName;
    m_strLoginName=TcpClient::getInstance().getLoginName();//获得登录名字
}

void PrivateChat::updateMsg(const PDU *pdu)
{

    if(nullptr==pdu)
    {
        qInfo()<<"updataMsg pdu is nullptr";
        return ;
    }
    char caSendName[32]={'\0'};
    memcpy(caSendName,pdu->caData,32);
    QString strMsg=QString("%1 says: %2").arg(caSendName).arg((char*)pdu->caMsg);
    ui->showMsg_te->append(strMsg);


}

void PrivateChat::on_sendMgs_PB_clicked()
{

    QString strMsg=ui->inputMsg_le->text();
    if(!strMsg.isEmpty())
    {
        PDU*pdu=mkPDU(strMsg.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;

        memcpy(pdu->caData,m_strLoginName.toUtf8().toStdString().c_str(),m_strLoginName.size()+1);
        memcpy(pdu->caData+32,m_strChatName.toUtf8().toStdString().c_str(),m_strChatName.size()+1);
        strcpy_s((char*)pdu->caMsg,strMsg.size()+1,strMsg.toUtf8().toStdString().c_str());

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;

    }
    else
    {

        //发送数据为空
        QMessageBox::warning(this,"私聊","发送的聊天信息不能为空");
    }
}

