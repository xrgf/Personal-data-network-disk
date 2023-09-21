#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include<QtSql/QSqlQuery>
class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    ~OpeDB();
    static OpeDB&getInstance();
    void init();
    bool handleRegist(const char *name, const char *pwd);
    bool handleLogin(const char *name, const char *pwd);
    void handleOffline(const char *name);
    QStringList hanleALLOnline();
    int handleSearchUsr(const char*name);
    int handleAddFriend(const char*pername,const char* name);
    void handleADDFriendAgree(const char*pername,const char*name);
    QStringList handleFlushFriend(const char*name);
    bool hanleDelFriend(const char*name,const char*friendName);

signals:
public slots:
private:
    QSqlDatabase m_db; //链接数据库


};

#endif // OPEDB_H
