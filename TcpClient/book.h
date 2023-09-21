﻿#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "protocol.h"

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
public slots:

    void createDir();
    void flushFile();
    void updateFileList(const PDU*pdu);
    void retUp();
    void enterLast();
    void delDir();
    void delFile();
    void uploadFile();

    void uploadFileData();

signals:

private:
    QListWidget *m_pBookListW;
    QPushButton*m_pReturnPB;
    QPushButton*m_pCreateDirPB;
    QPushButton*m_pDelDirPB;
    QPushButton*m_pRenamePB;
    QPushButton*m_pFlushFilePB;
    QPushButton*m_pUpLoadPB;
    QPushButton*m_pDownLoadPB;
    QPushButton*m_pDelFilePB;
    QPushButton*m_pShareFilePB;

    QString m_strUploadPath;
    QTimer*m_pTimer;

};

#endif // BOOK_H
