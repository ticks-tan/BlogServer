/**
* @Copyright(C) 2022.
*
* @filename: db_list.h
* @date: 2022/6/9
* @author: Ticks
* @description: 存储数据库链接的链表
*
* If you have contact or bugs, you can email to me, I can help you.
**/

#ifndef __BLOG_SERVER_DB_LIST_H__
#define __BLOG_SERVER_DB_LIST_H__

#include <condition_variable>
#include <mutex>
#include <mariadb/conncpp.hpp>

// 数据库链接链表
struct DBListNode
{
    // 当前节点是否被使用
    bool used_;
    // 上一个节点
    DBListNode* last_;
    // 下一个节点
    DBListNode* next_;
    // 存放当前连接
    std::unique_ptr<sql::Connection> con_;
};

class DBList final
{
public:
    DBList()
        : head_(nullptr)
        , tail_(nullptr)
    {
    }
    ~DBList();

    // 添加连接
    void addDBConnect(sql::Connection* con);
    // 使用(获取)链接
    DBListNode* useConnect();
    // 取消使用(归还)连接
    bool unUseConnect(DBListNode* con);

private:
private:
    DBListNode* head_;
    DBListNode* tail_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

#endif // __BLOG_SERVER_DB_LIST_H__
