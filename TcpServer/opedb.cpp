#include "opedb.h"
#include <QMessageBox>
OpeDB::OpeDB(QObject *parent)
    : QObject{parent}
{
    m_db=QSqlDatabase::addDatabase("QSQLITE");
    //init();
}

OpeDB::~OpeDB()
{
    m_db.close();
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("D:\\MyQtCode\\Qtcode\\TcpServer\\cloud.db");
    if(m_db.open())
    {

        QSqlQuery query;
        query.exec("select*from usrInfo");
        while(query.next())
        {
            QString data=QString("%1,%2,%3")
                               .arg(query.value(0).toString())
                               .arg(query.value(1).toString())
                               .arg(query.value(2).toString());
            qDebug()<<data;
        }
    }
    else
    {
        QMessageBox::critical(nullptr,"打开数据库失败","open db faild");
    }

}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if(nullptr==name||nullptr==pwd)
    {
        qInfo()<<"handleRegist function name or pwd is nullptr ";
        return false;
    }

    QString data=QString("insert into usrInfo(name,pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    //qDebug()<<data;
    QSqlQuery query;
    return query.exec(data);

}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if(nullptr==name||nullptr==pwd)
    {
       qInfo()<<"handleLogin function name or pwd is nullptr ";
        return false;
    }
    QString data=QString("select*from usrInfo where name=\'%1\'and pwd=\'%2\'and online=0").arg(name).arg(pwd);
    //qDebug()<<data;
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        QString data=QString(" update usrInfo set online=1 where name=\'%1\'and pwd=\'%2\'").arg(name).arg(pwd);
        //qDebug()<<data;
        QSqlQuery query;
        query.exec(data);//执行语句
        //qInfo()<<"+++++++++";
        return true;
    }
    else
    {
         //qDebug()<<"++++++++cccc+++++++";
        return false;
    }

}

void OpeDB::handleOffline(const char *name)
{
    if(nullptr==name)
    {
       qInfo()<<"handleOffline function name is nullptr ";
        return ;
    }
    QString data=QString("update usrInfo set online=0 where name=\'%1\'").arg(name);
    //qDebug()<<data;
    //qDebug()<<"我在这里测试";
    QSqlQuery query;
    query.exec(data);

}

QStringList OpeDB::hanleALLOnline()
{
    QString data=QString("select name from usrInfo where online=1");
    //qInfo()<<data;
    QSqlQuery query;
    query.exec(data);
    QStringList result;
    result.clear();
    //int i=0;
    while(query.next())
    {
        result<<QString("%1")
                      .arg(query.value(0).toString());
        //qInfo()<<result[i++];
    }
    return result;

}

int OpeDB::handleSearchUsr(const char *name)
{

    if(nullptr==name)
    {
        qInfo()<<"handleSearchUs function name is nullptr ";
        return -1 ;
    }
    QString data=QString("select online from usrInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        int ret=query.value("online").toInt();
        if(1==ret)
        {
            //表示在线
            return 1;
        }
        else if(0==ret)
        {
            //不在线
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if(nullptr==pername||nullptr==name)
    {

        qInfo()<<"handleAddFriend function name or pername is nullptr ";
        return -1;
    }
    //qInfo()<<"在这里测试数据库";
    QString data=QString("select*from friend where ( id=(select id from usrInfo where name=\'%1\') and friendId=(select id from usrInfo where name=\'%2\') ) "
                           "or  ( id=(select id from usrInfo where name=\'%3\') and friendId=(select id from usrInfo where name=\'%4\') )")
                       .arg(pername).arg(name).arg(name).arg(pername);
    //qDebug()<<data;

    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        return 0;//双发已经是好友
    }
    else
    {
        //不是好友
        //1 对方是否在线
        QString data=QString("select online from usrInfo where name=\'%1\'").arg(pername);
        QSqlQuery query;
        query.exec(data);
        if(query.next())
        {
            int ret=query.value("online").toInt();
            qInfo()<<"value"<<query.value("online").toInt();
            qDebug()<<"在这里测试ret"<<ret;

            if(1==ret)
            {
                //表示在线
                return 1;
            }
            else if(0==ret)
            {
                //不在线
                return 2;
            }
        }
        else
        {
            //不存在这个人

            //
            return 3;
        }
    }

}

void OpeDB::handleADDFriendAgree(const char *pername, const char *name)
{
    if(nullptr==pername||nullptr==name)
    {
        qInfo()<<"handleADDFriendAgree function name or pername is nullptr ";
        return ;
    }
    QString data=QString("insert into friend (id,friendId) values( (select id from usrInfo where name=\'%1\') "
                           ",(select id from usrInfo where name=\'%2\')) ").arg(pername).arg(name);
    qDebug()<<data;
    QSqlQuery query;
    query.exec(data);



}

QStringList OpeDB::handleFlushFriend(const char *name)
{
    QStringList strFriendList;
    if(nullptr==name)
    {
        qInfo()<<"hanleFlushFriend function name is nullptr ";
        return strFriendList;
    }
    QString data=QString("select name from usrInfo "
                           "where id IN  ( select friendId from friend where id=(select id from usrInfo where name=\'%1\') )"
                           "AND online=1").arg(name);
    QSqlQuery query;
    query.exec(data);
    while(query.next())
    {
        //qInfo()<<"test one";
        qInfo()<<query.value("name").toString();
        strFriendList.append(query.value("name").toString());
    }
    data.clear();
    data=QString("select name from usrInfo "
                           "where id IN  ( select id from friend where friendId=(select id from usrInfo where name=\'%1\') )"
                           "AND online=1").arg(name);

    query.exec(data);
    while(query.next())
    {
        //qInfo()<<"test two";
        qInfo()<<query.value("name").toString();
        strFriendList.append(query.value("name").toString());
    }
    return strFriendList;

}

bool OpeDB::hanleDelFriend(const char *name, const char *friendName)
{
    if(nullptr==friendName||nullptr==name)
    {
        qInfo()<<"hanleDelFriend function name or friendNamename is nullptr ";
        return false ;
    }
    QString data=QString("delete from friend where id=(select id from usrInfo where name=\'%1\') "
                           "and friendId=(select id from usrInfo where name=\'%2\')").arg(name).arg(friendName);
    QSqlQuery query;
    query.exec(data);
    data.clear();
    data=QString("delete from friend where friendId=(select id from usrInfo where name=\'%1\') "
                   "and id=(select id from usrInfo where name=\'%2\')").arg(name).arg(friendName);
     query.exec(data);

    return true;
}
