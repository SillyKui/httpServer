/*
 * @Author       : mark
 * @Date         : 2020-06-16
 * @copyleft Apache 2.0
 */ 
#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

class SqlConnPool {
public:
    static SqlConnPool *Instance();

    MYSQL *GetConn();       //得到mysql连接，返回mysql引用
    void FreeConn(MYSQL * conn);        //释放mysql连接
    int GetFreeConnCount();     //返回空闲的连接数量

    void Init(const char* host, int port,
              const char* user,const char* pwd, 
              const char* dbName, int connSize);
    void ClosePool();       //关闭数据库连接池

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;      //最大的用户连接数
    int useCount_;      //当前用户数
    int freeCount_;     //空闲用户数

    std::queue<MYSQL *> connQue_;       //队列（MySQL）指针
    std::mutex mtx_;            //互斥锁
    sem_t semId_;               //信号量
};


#endif // SQLCONNPOOL_H