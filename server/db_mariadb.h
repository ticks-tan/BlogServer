/**
* @Copyright(C) 2022.
*
* @filename: db_mariadb.h
* @date: 2022/6/9
* @author: Ticks
* @description: mariadb连接池和其他封装
*
* If you have contact or bugs, you can email to me, I can help you.
**/

#ifndef __BLOG_SERVER_DB_MARIADB_H__
#define __BLOG_SERVER_DB_MARIADB_H__

// 数据库连接链表
#include "db_list.h"

// 数据库连接池
class DBPool final
{
public:
    class DBConnection final
    {
        friend class DBPool;

    public:
        DBConnection(const DBConnection& connection)
        {
            this->node_ = connection.node_;
            this->pool_ = connection.pool_;
            this->canUse = connection.canUse;
        }
        DBConnection(DBConnection&& connection) noexcept
        {
            this->node_ = connection.node_;
            this->pool_ = connection.pool_;
            this->canUse = connection.canUse;
            connection.canUse = false;
            connection.pool_ = nullptr;
            connection.node_ = nullptr;
        }

        ~DBConnection()
        {
            if (this->canUse){
                this->pool_->revertConnection(*this);
            }
        }

    public:
        sql::Connection* get()
        {
            return this->node_->con_.get();
        }
    public:
        explicit operator sql::Connection* ()
        {
            return this->node_->con_.get();
        }

        sql::Connection* operator -> ()
        {
            return this->node_->con_.get();
        }

        sql::Connection& operator * ()
        {
            return *(this->node_->con_);
        }

    private:
        explicit DBConnection(DBPool* pool, DBListNode* node)
            : node_(node)
            , pool_(pool)
            , canUse(true)
        {
        }
        DBConnection() = default;

    private:
        DBListNode* node_;
        DBPool* pool_;
        bool canUse;
    };

public:
    ~DBPool();
public:
    // instance
    static std::shared_ptr<DBPool> newInstance(const sql::SQLString& url, const sql::Properties& properties, size_t count);
    // 获取一条连接
    __attribute__((unused)) DBConnection getConnection();
    // 归还一条连接
    bool revertConnection(DBConnection& connection);

    // 获取连接池url
    sql::SQLString& url()
    {
        return this->url_;
    }

    // 获取连接选项
    sql::Properties& properties()
    {
        return this->properties_;
    }

    // 获取当前连接池连接数量
    size_t count() const
    {
        return this->count_;
    }

private:
    // 私有构造函数，不能直接构造，只能通过 newInstance 构造
    DBPool(const sql::SQLString& url, const sql::Properties& properties, size_t count);
private:
    // 数据库地址
    sql::SQLString url_;
    // 数据库连接选项
    sql::Properties properties_;
    // mariadb driver
    sql::Driver* driver_;
    // 存储连接的链表
    DBList list_;
    // 连接池大小
    size_t count_;
};

class MariaDB_Exception : public std::exception
{
public:
    const char* what() const noexcept override
    {
        return this->error_;
    }
    explicit MariaDB_Exception(const char* error = "")
        : error_(error)
    {
    }
private:
    const char* error_;
};


#endif // __BLOG_SERVER_DB_MARIADB_H__
