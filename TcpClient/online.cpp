#include "online.h"
#include "ui_online.h"
#include <string.h>
#include <stdio.h>
#include <QMessageBox>
#include "tcpclient.h"
Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
    connect(ui->add_friend,&QPushButton::clicked,this,&Online::on_add_friend_clicked);
}

Online::~Online()
{
    delete ui;
}

void Online::showUsr(PDU *pdu)
{
    if(nullptr==pdu)
    {
        return;
    }
    uint uiSize=pdu->uiMsgLen/32;
    //qInfo()<<"uiSize"<<uiSize;
    char caTmp[32];
    //测试在线人
    //for(uint i=0;i<uiSize;i++)
    //{
    //    qInfo()<<"传入的消息是否正确";
    //    qInfo()<<(char*)pdu->caMsg+i*32;

    //}

    for(uint i=0;i<uiSize;i++)
    {
        memcpy(caTmp,(char*)pdu->caMsg+i*32,32);//pdu->caMsg+i*32注意这个地址偏移 因为是整形 所有实际移动字节数是i*32*4
        //strcpy_s(caTmp,32,)
        //qInfo()<<"在这里测试caTmp";
        //qInfo()<<caTmp;
        int flag=0;
        for(int i=0;i<ui->online_LW->count();i++)
        {
            if(ui->online_LW->item(i)->text().compare(caTmp))
            {
                flag++;//这样能保证里面有相同的就用插入了

            }
        }

        //qDebug()<<"ui-online_lw"<<ui->online_LW->count()<<"flag"<<flag;
        if(ui->online_LW->count()==flag)
        {
            ui->online_LW->addItem(caTmp);
        }
        memset(caTmp,0,0);


    }

}

void Online::on_add_friend_clicked()
{

    auto pItem=ui->online_LW->currentItem();
    if(pItem)
    {
        QString strUsrName=pItem->text();//加好友的名字
        QString strLoginName=TcpClient::getInstance().getLoginName();//自己的名字
        PDU*pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
        memcpy(pdu->caData,strUsrName.toUtf8().toStdString().c_str(),strUsrName.size());
        memcpy(pdu->caData+32,strLoginName.toUtf8().toStdString().c_str(),strLoginName.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;
    }
    else
    {
        QMessageBox::information(this,"添加好友","no select people");
    }

}

