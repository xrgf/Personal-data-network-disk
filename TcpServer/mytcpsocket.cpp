#include "mytcpsocket.h"
#include <QDebug>
#include "opedb.h"
#include <QStringList>
#include <string.h>
#include "mytcpserver.h"
#include <QDir>
#include <qdebug.h>
#include <QFileInfoList>
#include <QMessageBox>
MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{

    connect(this,&QTcpSocket::readyRead,this,&MyTcpSocket::recvMsg);
    //让每个客服端自己处理自己
    connect(this,&QTcpSocket::disconnected,this,&MyTcpSocket::clientOffiline);
    m_bUpload=false;

}

QString &MyTcpSocket::getm_strName()
{
    return this->m_strName;
}

void MyTcpSocket::flushList(PDU *pdu)
{

    char*pCurPath=new char[pdu->uiMsgLen+1];
    memcpy(pCurPath,pdu->caMsg,pdu->uiMsgLen);

    QDir dir(pCurPath);
    QFileInfoList fileInfoList=dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
    int iFileCount=fileInfoList.size();
    PDU*respdu=mkPDU(sizeof(FileInfo)*iFileCount);
    respdu->uiMsgType=ENUM_MSG_TYPE_FLUSh_FILE_RESPOND;
    FileInfo*pFileInfo=nullptr;
    QString strFileName;
    for(int i=0;i<fileInfoList.size();i++)
    {
        pFileInfo=(FileInfo*)(respdu->caMsg)+i;
        strFileName=fileInfoList[i].fileName();
        memcpy(pFileInfo->caFileName,strFileName.toUtf8().toStdString().c_str(),strFileName.size()+1);//写进去respdu
        if(fileInfoList[i].isDir())
        {
            pFileInfo->iFileType=0;//文件夹
        }
        else if(fileInfoList[i].isFile())
        {
            pFileInfo->iFileType=1;//非文件夹
        }


        qInfo()<<fileInfoList[i].fileName()
                <<fileInfoList[i].size()
                <<"文件夹"<<fileInfoList[i].isDir()
                <<"常规文件"<<fileInfoList[i].isFile();

    }
    write((char*)respdu,respdu->uiPDULen);
    free(respdu);
    respdu=nullptr;

}

void MyTcpSocket::recvMsg()
{
    if(!m_bUpload)
    {


        qDebug()<<this->bytesAvailable();
        //qDebug()<<"**************";
        uint uiPDULen=0;//??为什么能读到大小  答:结构体第一个
        this->read((char*)&uiPDULen,sizeof(uint));//读到uiPDULen.
        uint uiMsgLen=uiPDULen-sizeof(PDU);       //分二次去读取
        PDU*pdu=mkPDU(uiMsgLen);
        this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));

        switch(pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char caName[32]={'\0'};
            char caPwd[32]={'\0'};
            strcpy_s(caName,32,pdu->caData);
            strcpy_s(caPwd,32,pdu->caData+32);
             PDU*respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_REGIST_RESPOND;
            if(OpeDB::getInstance().handleRegist(caName,caPwd))
            {
                //注册成功
                strcpy_s(respdu->caData,32,REGIST_OK);
                QDir dir;
                qDebug()<<"create dir"<<dir.mkdir(QString("./%1").arg(caName));
            }
            else
            {
                strcpy_s(respdu->caData,32,REGIST_FAILED);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            free(pdu);
            pdu=nullptr;
            respdu=nullptr;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {

            char caName[64]={'\0'};
            char caPwd[64]={'\0'};
            strcpy_s(caName,32,pdu->caData);
            strcpy_s(caPwd,32,pdu->caData+32);
            PDU*respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_LOGIN_RESPOND;
            bool ret=OpeDB::getInstance().handleLogin(caName,caPwd);
            if(ret)
            {
                strcpy_s(respdu->caData,32,LOGIN_OK);
                m_strName=caName;
            }
            else
            {
                strcpy_s(respdu->caData,32,LOGIN_FAILED);
            }
            //qDebug()<<"m_strName"<<m_strName;
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            //free(pdu);
            //pdu=nullptr;
            respdu=nullptr;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            //qInfo()<<"*****";
            QStringList ret=OpeDB::getInstance().hanleALLOnline();
            uint uiMsgLen=ret.size()*32;//*4的原因是保证这里和客户端是一样的内存大小 方便操作 因为procotol里面是申请Msglen是单个字节的
            PDU*respdu=mkPDU(uiMsgLen);
            respdu->uiMsgType=ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for(int i=0;i<ret.size();i++)
            {
                 strcpy_s((char*)respdu->caMsg+i*32
                         ,ret.at(i).size()+1,(char*)ret.at(i).toUtf8().toStdString().c_str());
                //memcpy((char*)(respdu->caMsg+32*i),ret.at(i).toUtf8().toStdString().c_str(),ret.at(i).size()+1);
                //qInfo()<<"*********这里测试********";
                 respdu->caMsg[ret.at(i).size()+1+i*32]='\0';
                //qInfo()<<ret.at(i).toStdString().c_str();
                //qInfo()<<(char*)(respdu->caMsg+i*32);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;
            //qInfo()<<"*********这里测试********";
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
        {
            int ret=OpeDB::getInstance().handleSearchUsr(pdu->caData);
            PDU*respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
            if(-1==ret)
            {
                 strcpy_s(respdu->caData,64,SEARCH_USR_NO);
            }
            else if(0==ret)
            {
                 strcpy_s(respdu->caData,64,SEARCH_USR_OFFLINE);
            }
            else
            {
                 strcpy_s(respdu->caData,64,SEARCH_USR_ONLINE);
            }

            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;

            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            //加好友操作
            //1判断是否已经是好友
            //2判断是否在线
            //3在线处理
            char caPerName[64]={'\0'};
            char caName[64]={'\0'};
            strcpy_s(caPerName,32,pdu->caData);
            strcpy_s(caName,32,pdu->caData+32);
            int ret=OpeDB::getInstance().handleAddFriend(caPerName,caName);
            PDU*respdu=nullptr;

            if(-1==ret)
            {
                 respdu=mkPDU(0);
                 respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                 strcpy_s(respdu->caData,32,UNKNOW_ERROR);
                 write((char*)respdu,respdu->uiPDULen);
                 free(respdu);
                 respdu=nullptr;
            }
            else if(0==ret)
            {
                 respdu=mkPDU(0);
                 respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                 strcpy_s(respdu->caData,32,EXIST_FRIEND);
                  write((char*)respdu,respdu->uiPDULen);
                 free(respdu);
                 respdu=nullptr;
            }
            else if(1==ret)
            {
                 MyTcpServer::getInstance().resend(caPerName,pdu);
            }
            else if(2==ret)
            {
                 respdu=mkPDU(0);
                 respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                 strcpy_s(respdu->caData,32, ADD_FRIEND_OFFLINE);
                  write((char*)respdu,respdu->uiPDULen);
                 free(respdu);
                 respdu=nullptr;
            }
            else if(3==ret)
            {
                 respdu=mkPDU(0);
                 respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                 strcpy_s(respdu->caData,32,ADD_FRIEND_NOEXIST);
                 write((char*)respdu,respdu->uiPDULen);
                 free(respdu);
                 respdu=nullptr;
            }



            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE:
        {
            //好友同意就添加到数据库
            char caName[32]={'\0'};//自己名字
            char caPerName[32]={'\0'};//对方名字
            strcpy_s(caPerName,32,pdu->caData);
            strcpy_s(caName,32,pdu->caData+32);
            OpeDB::getInstance().handleADDFriendAgree(caName,caPerName);

            PDU*toclientpdu=mkPDU(0);
            toclientpdu->uiMsgType=ENUM_MSG_TYPE_FRIEND_AGREE_RESPOND;
            MyTcpServer::getInstance().resend(caName,toclientpdu);
            free(toclientpdu);
            toclientpdu=nullptr;

            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            //好友同意就添加到数据库
            char caName[32]={'\0'};//自己名字
            char caPerName[32]={'\0'};//对方名字
            strcpy_s(caPerName,32,pdu->caData);
            strcpy_s(caName,32,pdu->caData+32);

            PDU*toclientpdu=mkPDU(0);
            toclientpdu->uiMsgType=ENUM_MSG_TYPE_FRIEND_REFUSE_RESPOND;
            MyTcpServer::getInstance().resend(caName,toclientpdu);
            free(toclientpdu);
            toclientpdu=nullptr;

        }
        case ENUM_MSG_TYPE_FLUSE_FRIEND_REQUEST:
        {
            char caName[32]={'\0'};
            strcpy_s(caName,32,pdu->caData);
            QStringList ret=OpeDB::getInstance().handleFlushFriend(pdu->caData);
            uint uiMsgLen=ret.size()*32;
            PDU*respdu=mkPDU(uiMsgLen);
            respdu->uiMsgType=ENUM_MSG_TYPE_FLUSE_FRIEND_RESPOND;
            for(int i=0;i<ret.size();i++)
            {
                 memcpy((char*)(respdu->caMsg)+i*32,ret.at(i).toUtf8().toStdString().c_str(),ret.at(i).size());
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caSelgName[32]={'\0'};
            char caFriendName[32]={'\0'};
            strcpy_s(caSelgName,32,pdu->caData);
            strcpy_s(caFriendName,32,pdu->caData+32);
            OpeDB::getInstance().hanleDelFriend(caSelgName,caFriendName);

            PDU*respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            strcpy_s(respdu->caData,64,DEL_FRIEND_OK);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;

            //转发删除好友信息
            MyTcpServer::getInstance().resend(caFriendName,pdu);//给朋友回复 删除好友了



            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char caPerName[32]={'\0'};
            memcpy(caPerName,pdu->caData+32,32);
            qDebug()<<caPerName;
            MyTcpServer::getInstance().resend(caPerName,pdu);//服务器转发

        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            //查找在线好友
            char caName[32]={'\0'};
            strcpy_s(caName,32,pdu->caData);
            QStringList onlineFriend=OpeDB::getInstance().handleFlushFriend(caName);
            for(int i=0;i<onlineFriend.size();i++)
            {
                 auto temp=onlineFriend.at(i);
                 MyTcpServer::getInstance().resend((char*)temp.toUtf8().toStdString().c_str(),pdu);
            }

            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QDir dir;
            QString strCurPath=QString("%1").arg((char*)(pdu->caMsg));
            bool ret=dir.exists(strCurPath);
            PDU*respdu=nullptr;
            if(ret)
            {
                 //当前目录存在
                 char caNewDir[32]={'\0'};
                 memcpy(caNewDir,pdu->caData+32,32);
                 QString strNewPath=strCurPath+"/"+caNewDir;
                 qDebug()<<strNewPath;
                 ret=dir.exists(strNewPath);
                 qDebug()<<"-->"<<ret;
                 if(ret)//如果文件名存在
                 {
                     respdu=mkPDU(0);
                     respdu->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                     strcpy_s(respdu->caData,FILE_NAME_EXIST);


                 }
                 else//文件名不存在
                 {
                     dir.mkdir(strNewPath);
                     respdu=mkPDU(0);
                     respdu->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                     strcpy_s(respdu->caData,CREATE_DIR_OK);

                 }
            }
            else
            {
                 //当前目录不存在 //就是没有当前路径创建不了
                 respdu=mkPDU(0);
                 respdu->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                 strcpy_s(respdu->caData,DIR_NOT_EXITST);

            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;
            break;
        }
        case ENUM_MSG_TYPE_FLUSh_FILE_REQUEST:
        {
            flushList(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_DIR_REQUEST:
        {

            char caName[64]={'\0'};
            strcpy_s(caName,64,pdu->caData);//文件名

            char*pPath=new char[pdu->uiMsgLen+1];
            memcpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);
            QString strPath=QString("%1/%2").arg(pPath).arg(caName);
            qInfo()<<"strPath"<<strPath;
            QFileInfo fileInfo(strPath);
            bool ret=false;
            if(fileInfo.isDir())
            {
                 QDir dir;
                 dir.setPath(strPath);
                 dir.removeRecursively();//递归删除文件夹里面所有文件
                 ret=true;
            }
            else if(fileInfo.isFile())//常规文件
            {
                 ret =false;
            }
            PDU*respdu=nullptr;
            if(ret)
            {
                 respdu=mkPDU(QString(DEL_DIR_OK).size()+1);
                 respdu->uiMsgType=ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
                 memcpy(respdu->caMsg,DEL_DIR_OK,respdu->uiMsgLen);
            }
            else
            {
                 respdu=mkPDU(strlen(DEL_DIR_FAILED)+1);
                 respdu->uiMsgType=ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
                 memcpy((char*)respdu->caMsg,DEL_DIR_FAILED,respdu->uiMsgLen);

                 qInfo()<<(char*)respdu->caMsg<<respdu->uiMsgLen<<strlen(DEL_DIR_FAILED)+1;
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;

            break;
        }
        case ENUM_MSG_TYPE_DELETE_FILE_REQUEST:
        {
            char caName[64]={'\0'};
            strcpy_s(caName,64,pdu->caData);//文件名


            char*pPath=new char[pdu->uiMsgLen+1];
            memcpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);
            QString strPath=QString("%1/%2").arg(pPath).arg(caName);
            qInfo()<<"strPath"<<strPath;
            QFileInfo fileInfo(strPath);

            bool ret=false;
            if(fileInfo.isFile())
            {
                 QFile file(strPath);

                 file.remove();
                 ret=true;
            }
            else if(fileInfo.isDir())//文件夹
            {
                 ret =false;
            }
            PDU*respdu=nullptr;
            if(ret)
            {
                 respdu=mkPDU(QString(DEL_FILE_OK).size()+1);
                 respdu->uiMsgType=ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
                 memcpy(respdu->caMsg,DEL_FILE_OK,respdu->uiMsgLen);
            }
            else
            {
                 respdu=mkPDU(strlen(DEL_FILE_FAILED)+1);
                 respdu->uiMsgType=ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
                 memcpy((char*)respdu->caMsg,DEL_FILE_FAILED,respdu->uiMsgLen);

                 qInfo()<<(char*)respdu->caMsg<<respdu->uiMsgLen<<strlen(DEL_FILE_FAILED)+1;
            }
            delete []pPath;
            pPath=nullptr;
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;

        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESQUEST:
        {

            qInfo()<<"test entry upload file request";
            char caFileName[32]={'\0'};
            qint64 fileSize=0;
            qInfo()<<"test one";
            std::sscanf(pdu->caData,"%s %lld",caFileName,&fileSize);
            qInfo()<<"test sscanf";
            char*pPath=new char[pdu->uiMsgLen+1];
            qInfo()<<"test pPath";
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen+1);
            qInfo()<<"pPath"<<pPath<<"caFileName"<<caFileName;
            QString strPath=QString("%1/%2").arg(pPath).arg(caFileName);
            qInfo()<<strPath<<"strPath";
            delete []pPath;
            pPath=nullptr;
            qInfo()<<"test fileSize"<<fileSize;
            m_file.setFileName(strPath);

            if(m_file.open(QIODevice::WriteOnly))
            {
                 qInfo()<<"entry m_file";
                 m_bUpload=true;
                 m_iTotal=fileSize;
                 qInfo()<<"test m_iTotal"<<m_iTotal;
                 m_iRecved=0;
            }
            break;
        }

        free(pdu);
        pdu=nullptr;
        default:
            break;

        }
    }
    else
    {
        //上传文件状态
        qInfo()<<"entry else";
        PDU*respdu=nullptr;
        QByteArray buff=readAll();
        qInfo()<<"test write";
        m_file.write(buff);
        m_iRecved+=buff.size();
        if(m_iTotal==m_iRecved)
        {
            qInfo()<<"success";

            m_file.close();
            m_bUpload=false;
            respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            strcpy_s(respdu->caData,strlen(UPLOAD_FILE_OK)+1,UPLOAD_FILE_OK);
             qInfo()<<"test colse one";

        }
        else if(m_iTotal<m_iRecved)
        {
             qInfo()<<"test failed";
            m_file.close();
            m_bUpload=false;
            respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            strcpy_s(respdu->caData,strlen(UPLOAD_FILE_FAILED)+1,UPLOAD_FILE_FAILED);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=nullptr;
        qInfo()<<"test colse";

    }



    //qDebug()<<caName<<caPwd<<pdu->uiMsgType;
    //qDebug()<<pdu->uiMsgType<<(char*)(pdu->caMsg);

}

void MyTcpSocket::clientOffiline()
{
    OpeDB::getInstance().handleOffline(m_strName.toUtf8().toStdString().c_str());
    emit offline(this);
}
