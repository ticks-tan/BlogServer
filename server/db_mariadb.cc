/**
* @Copyright(C) 2022.
*
* @filename: db_mariadb.cc
* @date: 2022/6/11
* @author: Ticks
* @description:
*
* If you have contact or bugs, you can email to me, I can help you.
**/

#include "db_mariadb.h"

std::shared_ptr<DBPool> DBPool::newInstance(const sql::SQLString& url, const sql::Properties& properties, size_t count)
{
    return std::shared_ptr<DBPool>(new DBPool(url, properties, count));
}

DBPool::DBPool(const sql::SQLString &url, const sql::Properties &properties, size_t count)
    : url_(url)
    , properties_(properties)
    , count_(count)
{
    if (url.empty() || properties.empty()){
        throw MariaDB_Exception("url or properties is null!");
    }
    this->driver_ = sql::mariadb::get_driver_instance();
    sql::Connection* con;
    if (this->driver_ != nullptr) {
        while (count > 0) {
            con = this->driver_->connect(this->url_, this->properties_);
            if (con != nullptr) {
                this->list_.addDBConnect(con);
            } else{
                --this->count_;
            }
            --count;
        }
    } else{
        this->count_ = 0;
    }
}

DBPool::~DBPool()
{
    this->count_ = 0;
}

__attribute__((unused))
DBPool::DBConnection DBPool::getConnection()
{
    DBConnection connection(this, nullptr);
    if (this->driver_ == nullptr || this->count_ == 0){
        return std::move(connection);
    }
    auto con = this->list_.useConnect();
    if (con != nullptr){
        connection.node_ = con;
        return std::move(connection);
    }
    return std::move(connection);
}

bool DBPool::revertConnection(DBPool::DBConnection& connection)
{
    if (this->driver_ == nullptr || this->count_ == 0 || connection.get() == nullptr) {
        return false;
    }
    bool re = this->list_.unUseConnect(connection.node_);
    connection.canUse = false;
    return re;
}
