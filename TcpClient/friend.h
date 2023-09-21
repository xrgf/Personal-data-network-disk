#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "online.h"
#include "protocol.h"
#include "privatechat.h"
class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllOnluneUsr(PDU*pdu);
    void updateFriendList(PDU*pdu);
    void updateGroupMsg(PDU*pdu);

    QString m_strSearchName;

signals:
public slots:
    void showOnline();
    void searchUsr();
    void flushFriend();

    void delFriend();
    void privateChat();
    void groupChat();
private:
    QTextEdit*m_pShowMsgTE;            //显示消息
    QListWidget*m_pFriendListWidget;  //显示好友列表
    QLineEdit*m_pInputMsgLE;          //信息输入框

    QPushButton*m_pDelFriendPB;
    QPushButton*m_pFlushFriendPB;
    QPushButton*m_pShowOnlineUsrPB;
    QPushButton*m_pSearchUsrPB;
    QPushButton*m_pMsgSendPB;
    QPushButton*m_pPrivateChatPB;
    Online*m_pOnline;




};

#endif // FRIEND_H
