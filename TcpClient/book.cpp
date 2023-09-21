#include "book.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>
#include <string.h>
#include <QFileDialog>
#include <QTimer>
Book::Book(QWidget *parent)
    : QWidget{parent}
{

    m_pTimer=new QTimer;
    m_pBookListW=new QListWidget ;
    m_pReturnPB=new QPushButton("返回")  ;
    m_pCreateDirPB=new QPushButton("创建文件夹");
    m_pDelDirPB   =new QPushButton("删除文件夹");
    m_pRenamePB   =new QPushButton("重命名文件");
    m_pFlushFilePB=new QPushButton("刷新文件");


    QVBoxLayout*pDirVBL=new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB );
    pDirVBL->addWidget(m_pFlushFilePB);

    m_pUpLoadPB   =new QPushButton("上传文件");
    m_pDownLoadPB =new QPushButton("下载文件");
    m_pDelFilePB  =new QPushButton("删除文件");
    m_pShareFilePB=new QPushButton("分享文件");

    QVBoxLayout*pFileVBL=new QVBoxLayout;
    pFileVBL->addWidget(m_pUpLoadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);

    QHBoxLayout*pMain=new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);
    flushFile();
    setLayout(pMain);

    connect(m_pCreateDirPB,&QPushButton::clicked,this,&Book::createDir);
    connect(m_pFlushFilePB,&QPushButton::clicked,this,&Book::flushFile);
    connect(m_pBookListW,&QListWidget::doubleClicked,this,&Book::enterLast);
    connect(m_pReturnPB,&QPushButton::clicked,this,&Book::retUp);
    connect(m_pDelDirPB,&QPushButton::clicked,this,&Book::delDir);
    connect(m_pDelFilePB,&QPushButton::clicked,this,&Book::delFile);
    connect(m_pUpLoadPB,&QPushButton::clicked,this,&Book::uploadFile);

    connect(m_pTimer,&QTimer::timeout,this,&Book::uploadFileData);

}

void Book::createDir()
{
    QString strNewDir=QInputDialog::getText(this,"新建文件夹","新文件夹名字");
    if(!strNewDir.isEmpty())
    {
        if(strNewDir.size()>32)
        {
            QMessageBox::warning(this,"新建文件夹","新建文件夹名太长");
        }
        else
        {
            QString strName=TcpClient::getInstance().getLoginName();
            QString strCurPath=TcpClient::getInstance().getCurPath();
            PDU*pdu=mkPDU(strCurPath.size()+1);
            pdu->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strcpy_s(pdu->caData,strName.size()+1,strName.toUtf8().toStdString().c_str());
            strcpy_s(pdu->caData+32,strNewDir.size()+1,strNewDir.toUtf8().toStdString().c_str());
            memcpy((char*)pdu->caMsg,strCurPath.toUtf8().toStdString().c_str(),strCurPath.size()+1);
            //qInfo()<<"strCurPath test "<<strCurPath;
            //qInfo()<<"strNewName test"<<strNewDir;
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu=nullptr;
        }


    }
    else
    {
          QMessageBox::warning(this,"新建文件夹","新文件夹名字不能为空");

    }


    //PDU*pdu=mkPDU();
}

void Book::flushFile()
{
    QString strCurPath=TcpClient::getInstance().getCurPath();
    PDU*pdu=mkPDU(strCurPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_TYPE_FLUSh_FILE_REQUEST;
    strcpy_s((char*)pdu->caMsg,strCurPath.size()+1,strCurPath.toUtf8().toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=nullptr;
}

void Book::updateFileList(const PDU *pdu)
{
    if(nullptr==pdu)
    {
          qInfo()<<"updateFileList pdu is nullptr";
          return ;
    }
    FileInfo *pFileInfo=nullptr;
    int iCount=pdu->uiMsgLen/sizeof(FileInfo);

    m_pBookListW->clear();//防止二次刷新残留
    for(int i=0;i<iCount;i++)
    {

          pFileInfo=(FileInfo*)(pdu->caMsg)+i;
          //qInfo()<<"test";
           QListWidgetItem*pItem=new QListWidgetItem;
          //qInfo()<<pFileInfo->caFileName<<pFileInfo->iFileType;

          if(0==pFileInfo->iFileType)
          {
            pItem->setIcon(QIcon(QPixmap(":/map/dir.jpg")));
          }
          else if(1==pFileInfo->iFileType)
          {
            qInfo()<<"pFileInfo->iFileType"<<pFileInfo->iFileType;
            pItem->setIcon(QIcon(":/map/reg.jpg"));
          }
          pItem->setText(pFileInfo->caFileName);
          m_pBookListW->addItem(pItem);
    }
}

void Book::retUp()
{
    QString strCurPath=TcpClient::getInstance().getCurPath();
    size_t pos=strCurPath.lastIndexOf('/');
    strCurPath.erase(strCurPath.begin()+pos,strCurPath.end());
    qInfo()<<"test";
    qInfo()<<strCurPath;
    TcpClient::getInstance().updateCurPath(strCurPath);
    PDU*pdu=mkPDU(strCurPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_TYPE_FLUSh_FILE_REQUEST;
    strcpy_s((char*)pdu->caMsg,strCurPath.size()+1,strCurPath.toUtf8().toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=nullptr;

}

void Book::enterLast()
{
    QString curNewName=m_pBookListW->currentItem()->text();
    QString strCurPath=TcpClient::getInstance().getCurPath();
    strCurPath=strCurPath+"/"+curNewName;
    TcpClient::getInstance().updateCurPath(strCurPath);
    PDU*pdu=mkPDU(strCurPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_TYPE_FLUSh_FILE_REQUEST;
    strcpy_s((char*)pdu->caMsg,strCurPath.size()+1,strCurPath.toUtf8().toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=nullptr;

}

void Book::delDir()
{

    auto pItem=m_pBookListW->currentItem();
    QString strCurPath=TcpClient::getInstance().getCurPath();
    if(nullptr==pItem)
    {
          QMessageBox::warning(this,"删除文件","请选择删除文件名字");
    }
    else
    {
          QString strDelName=pItem->text();
          PDU*pdu=mkPDU(strCurPath.size()+1);
          pdu->uiMsgType=ENUM_MSG_TYPE_DELETE_DIR_REQUEST;
          //strcpy_s((char*)pdu->caMsg,strCurPath.size(),strCurPath.toUtf8().toStdString().c_str());
          strcpy_s(pdu->caData,strDelName.size()+1,strDelName.toUtf8().toStdString().c_str());
          memcpy((char*)pdu->caMsg,strCurPath.toUtf8().toStdString().c_str(),strCurPath.size()+1);
          TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
          free(pdu);
          pdu=nullptr;
    }






}

void Book::delFile()
{
    auto pItem=m_pBookListW->currentItem();
    QString strCurPath=TcpClient::getInstance().getCurPath();
    if(nullptr==pItem)
    {
          QMessageBox::warning(this,"删除文件","请选择删除文件名字");
    }
    else
    {
          QString strDelName=pItem->text();
          PDU*pdu=mkPDU(strCurPath.size()+1);
          pdu->uiMsgType=ENUM_MSG_TYPE_DELETE_FILE_REQUEST;
          //strcpy_s((char*)pdu->caMsg,strCurPath.size(),strCurPath.toUtf8().toStdString().c_str());
          strcpy_s(pdu->caData,strDelName.size()+1,strDelName.toUtf8().toStdString().c_str());
          memcpy((char*)pdu->caMsg,strCurPath.toUtf8().toStdString().c_str(),strCurPath.size()+1);
          TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
          free(pdu);
          pdu=nullptr;
    }

}

void Book::uploadFile()
{
    QString strCurPath=TcpClient::getInstance().getCurPath();
    m_strUploadPath=QFileDialog::getOpenFileName();
    //qInfo()<<"strUploadFilePath"<<strUploadFilePath;
    if(!m_strUploadPath.isEmpty())
    {
          int index=m_strUploadPath.lastIndexOf('/');
          QString strFileName=m_strUploadPath.right(m_strUploadPath.size()-index-1);
          qInfo()<<strFileName;
          QFile file(m_strUploadPath);
          qint64 fileSize=file.size();//获得文件大小
          QString strCurPath=TcpClient::getInstance().getCurPath();
          PDU*pdu=mkPDU(strCurPath.size()+1);
          pdu->uiMsgType=ENUM_MSG_TYPE_UPLOAD_FILE_RESQUEST;
          memcpy((char*)pdu->caMsg,strCurPath.toUtf8().toStdString().c_str(),strCurPath.size());
          sprintf(pdu->caData,"%s %lld",strFileName.toUtf8().toStdString().c_str(),fileSize);
          TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
          free(pdu);
          pdu=nullptr;
          m_pTimer->start(1000);
    }
    else
    {
          QMessageBox::warning(this,"上传文件","上传文件不能为空");
    }

}

void Book::uploadFileData()
{
    qInfo()<<"entry uploadFileData";
    m_pTimer->stop();
    QFile file(m_strUploadPath);
    if(!file.open(QIODevice::ReadOnly))
    {
          QMessageBox::warning(this,"上传文件","上传文件失败");
          return ;
    }
    char*pBuffer=new char[4096];
    qint64 ret=0;
    while(true)
    {
          ret=file.read(pBuffer,4096);//ret 接收实际读到多少
          if(ret>0&&ret<=4096)
          {
            TcpClient::getInstance().getTcpSocket().write(pBuffer,ret);

          }
          else if(0==ret)
          {
            break;//读完
          }
          else
          {
            QMessageBox::warning(this,"上传文件","读文件失败");
            break;
          }
    }
    qInfo()<<"close";
    file.close();
    delete[]pBuffer;
    pBuffer=nullptr;
}




