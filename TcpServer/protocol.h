#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdlib.h>
#include <string.h>
typedef unsigned int uint;
#define REGIST_OK "regist ok"
#define REGIST_FAILED "regist failed :name existed"

#define LOGIN_OK "login ok"
#define LOGIN_FAILED "login failed"

#define SEARCH_USR_NO "no such people"
#define SEARCH_USR_ONLINE "online"
#define SEARCH_USR_OFFLINE "offline"

#define UNKNOW_ERROR "unknow error"
#define EXIST_FRIEND "friend exist"
#define ADD_FRIEND_OFFLINE "usr offline"
#define ADD_FRIEND_NOEXIST "usr not exist"

#define DEL_FRIEND_OK "delete friend ok"

#define DIR_NOT_EXITST "cur dir not exist"
#define FILE_NAME_EXIST "file name exist"
#define CREATE_DIR_OK   "create dir ok"

#define DEL_DIR_OK "del dir ok"
#define DEL_DIR_FAILED "del dir failed :is reguler file"

#define DEL_FILE_OK "del file ok"
#define DEL_FILE_FAILED "del file failed :not is reguler file"

#define  UPLOAD_FILE_OK "upload file ok"
#define UPLOAD_FILE_FAILED "upload file failed"
struct FileInfo
{
    char caFileName[32];//文件名字
    int iFileType;      //文件类型
};

enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN=0,
    ENUM_MSG_TYPE_REGIST_REQUEST,    //注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND,    //注册回复

    ENUM_MSG_TYPE_LOGIN_REQUEST,    //登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND,    //登录回复

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,//在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,//在线用户回复

    ENUM_MSG_TYPE_SEARCH_USR_REQUEST,//搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND,//搜索用户回复

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,//添加用户请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,//添加用户回复

    ENUM_MSG_TYPE_ADD_FRIEND_AGGREE, //同意添加好友
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE, //拒绝添加好友

    ENUM_MSG_TYPE_FRIEND_AGREE_RESPOND,//好友同意 回复
    ENUM_MSG_TYPE_FRIEND_REFUSE_RESPOND,//好友拒绝 回复

    ENUM_MSG_TYPE_FLUSE_FRIEND_REQUEST,//刷新好友请求
    ENUM_MSG_TYPE_FLUSE_FRIEND_RESPOND,//刷新好友回复


    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,//删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,//删除好友回复

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,//私聊好友请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,//私聊好友回复


    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,//群聊请求
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,//群聊回复

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,//创建文件夹请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,//创建文件夹回复

    ENUM_MSG_TYPE_FLUSh_FILE_REQUEST,//刷新文件夹请求
    ENUM_MSG_TYPE_FLUSh_FILE_RESPOND,//刷新文件夹回复

    ENUM_MSG_TYPE_LAST_FILE_REQUEST,//查看下一级请求
    ENUM_MSG_TYPE_LAST_FILE_RESPOND,//查看下一级回复

    ENUM_MSG_TYPE_UP_FILE_REQUEST,//查看上一级请求
    ENUM_MSG_TYPE_UP_FILE_RESPOND,//查看上一级回复

    ENUM_MSG_TYPE_DELETE_DIR_REQUEST,//删除文件夹请求
    ENUM_MSG_TYPE_DELETE_DIR_RESPOND,//删除文件夹回复

     ENUM_MSG_TYPE_DELETE_FILE_REQUEST,//删除文本文件请求
    ENUM_MSG_TYPE_DELETE_FILE_RESPOND,//删除文本文件回复

    ENUM_MSG_TYPE_UPLOAD_FILE_RESQUEST,//上传文件请求
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,//上传文件回复



    ENUM_MSG_TYPE_MAX=0X00ffffff
};
struct PDU
{
    uint uiPDULen;   //总的协议数据单元大小
    uint uiMsgType;  //消息类型（告诉对方数据是干嘛的）
    char caData[64];
    uint uiMsgLen;   //实际消息长度
    int caMsg[];     //实际消息
};
PDU* mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
