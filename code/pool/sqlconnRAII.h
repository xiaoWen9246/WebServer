

#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H
#include "sqlconnpool.h"

/* 资源在对象构造初始化 资源在对象析构时释放*/
class SqlConnRAII {
public:
    SqlConnRAII(MYSQL** sql, SqlConnPool *connpool) {  // 构造函数获取资源
        assert(connpool);
        *sql = connpool->GetConn();
        sql_ = *sql;
        connpool_ = connpool;
    }
    
    ~SqlConnRAII() {   // 析构函数释放资源
        if(sql_) { connpool_->FreeConn(sql_); }
    }
    
private:
    MYSQL *sql_;     
    SqlConnPool* connpool_;   // 连接池
};

#endif //SQLCONNRAII_H