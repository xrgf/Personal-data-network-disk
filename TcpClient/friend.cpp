#include "friend.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <string.h>
#include <stdio.h>
#include <QMessageBox>
#include <QString>
Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTE=new QTextEdit;            //显示消息
    m_pFriendListWidget=new QListWidget;  //显示好友列表
    m_pInputMsgLE=new QLineEdit;          //信息输入框

    m_pDelFriendPB=new QPushButton("删除好友");
    m_pFlushFriendPB=new QPushButton("刷新好友");
    m_pShowOnlineUsrPB=new QPushButton("显示在线用户");
    m_pSearchUsrPB=new QPushButton("查找用户");
    m_pMsgSendPB=new QPushButton("信息发送");
    m_pPrivateChatPB=new QPushButton("私聊");
    QVBoxLayout*pRightPB=new QVBoxLayout;
    pRightPB->addWidget(m_pDelFriendPB);
    pRightPB->addWidget(m_pFlushFriendPB);
    pRightPB->addWidget(m_pShowOnlineUsrPB);
    pRightPB->addWidget(m_pSearchUsrPB);
    pRightPB->addWidget(m_pPrivateChatPB);

    QHBoxLayout*pTopHBL=new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget( m_pFriendListWidget);
    pTopHBL->addLayout(pRightPB);

    QHBoxLayout *pMsgHBL=new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline=new Online;

    QVBoxLayout*pMain=new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();

    setLayout(pMain);

    connect(m_pShowOnlineUsrPB,&QPushButton::clicked,this,&Friend::showOnline);
    connect(m_pSearchUsrPB,&QPushButton::clicked,this,&Friend::searchUsr);
    connect(m_pFlushFriendPB,&QPushButton::clicked,this,&Friend::flushFriend);
    connect(m_pDelFriendPB,&QPushButton::clicked,this,&Friend::delFriend);
    connect(m_pPrivateChatPB,&QPushButton::clicked,this,&Friend::privateChat);
    connect(m_pMsgSendPB,&QPushButton::clicked,this,&Friend::groupChat);
}

void Friend::showAllOnluneUsr(PDU *pdu)
{
    if(nullptr==pdu)
    {
        return ;
    }
    m_pOnline->showUsr(pdu);

}

void Friend::showOnline()
{
    if(m_pOnline->isHidden())
    {
        m_pOnline->show();
        PDU*pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;

    }
    else
    {
        m_pOnline->hide();
    }
}

void Friend::searchUsr()
{
    QString name=QInputDialog::getText(this,"搜索","用户名");
    m_strSearchName=name;
    if(!name.isEmpty())
    {
        qDebug()<<name;
    }
    PDU*pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
    strcpy_s(pdu->caData,name.size()+1,name.toUtf8().toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=nullptr;
}

void Friend::flushFriend()
{
    QString strName=TcpClient::getInstance().getLoginName();
    PDU*pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_TYPE_FLUSE_FRIEND_REQUEST;
    memcpy(pdu->caData,strName.toUtf8().toStdString().c_str(),strName.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=nullptr;


}

void Friend::updateFriendList(PDU *pdu)
{
    if(nullptr==pdu)
    {
        return ;
    }
    uint uiSize=pdu->uiMsgLen/32;
    char caName[32]={'\0'};
    m_pFriendListWidget->clear();
    for(uint i=0;i<uiSize;i++)
    {
        memcpy(caName,(char*)(pdu->caMsg)+i*32,32);
        m_pFriendListWidget->addItem(caName);
        memset(caName,0,0);
    }
#if 0
    for(uint i=0;i<uiSize;i++)
    {
        //qInfo()<<"在这里测试 caName";

        memcpy(caName,(char*)(pdu->caMsg)+i*32,32);
        //qInfo()<<(char*)(pdu->caMsg)+i*32<<caName;
        int flag=0;
        for(int i=0;i<m_pFriendListWidget->count();i++)
        {
            if(m_pFriendListWidget->item(i)->text().compare(caName))
            {
                flag++;
            }
        }
        if(m_pFriendListWidget->count()==flag)
        {
             m_pFriendListWidget->addItem(caName);
            flag=0;
        }

        memset(caName,0,0);
    }
#endif
}

void Friend::updateGroupMsg(PDU *pdu)
{
    QString strMsg=QString(" %1 say: %2").arg(pdu->caData).arg((char*)pdu->caMsg);
    m_pShowMsgTE->append(strMsg);

}

void Friend::delFriend()
{
    if(nullptr!=m_pFriendListWidget->currentItem())
    {
        QString strFriendName=m_pFriendListWidget->currentItem()->text();//对方的名字
        //qDebug()<<strName;
        PDU*pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        QString strSelfName=TcpClient::getInstance().getLoginName();//自己的名字
        memcpy(pdu->caData,strSelfName.toUtf8().toStdString().c_str(),32);
        memcpy(pdu->caData+32,strFriendName.toUtf8().toStdString().c_str(),32);
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;

    }

}

void Friend::privateChat()
{
    //关联私聊按钮和私聊窗口
    if(nullptr!=m_pFriendListWidget->currentItem())
    {
        QString strChataName=m_pFriendListWidget->currentItem()->text();
        PrivateChat::getInstance().setChatName(strChataName);
        if(PrivateChat::getInstance().isHidden())
        {
            PrivateChat::getInstance().show();
        }


    }
    else
    {
        QMessageBox::warning(this,"私聊","请选择私聊对象");
    }

}

void Friend::groupChat()
{

    //首先获得聊天信息
    QString strMsg=m_pInputMsgLE->text();
    if(!strMsg.isEmpty())
    {
        PDU*pdu=mkPDU(strMsg.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        QString strName=TcpClient::getInstance().getLoginName();
        strcpy_s(pdu->caData,strName.size()+1,strName.toUtf8().toStdString().c_str());
        strcpy_s((char*)pdu->caMsg,strMsg.size()+1,strMsg.toUtf8().toStdString().c_str());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;
    }
    else
    {
        QMessageBox::warning(this,"群聊","信息不能为空");
    }
}
