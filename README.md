# Personal-data-network-disk
项目简介:该网盘采用C/S 架构，数据库存储，用户信息，磁盘存储用户文件，该网盘能够提供文件同步，备份以及共享等服务，通过网盘，用户可以多终端上传，管理。
开发环境:Qt Creator SQList3

#注意点是 保护协议动态结构体中消息定义是int 在每次转换中需要先转换类型在进行指针加操作
#上传文件目前还没有用多线程分割上传 上传大文件服务器会崩溃
#注意如果服务器先于客户端结束 数据库内就操作不到客户端 所以需要重新转换客户端的上下线状态

