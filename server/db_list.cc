/**
* @Copyright(C) 2022.
*
* @filename: db_list.cc
* @date: 2022/6/9
* @author: Ticks
* @description:
*
* If you have contact or bugs, you can email to me, I can help you.
**/

#include "db_list.h"

DBList::~DBList()
{
    DBListNode* node = this->head_, *next;
    while (node){
        next = node->next_;
        // 关闭连接
        if (node->con_){
            node->con_->close();
        }
        node->con_.reset();
        delete node;
        node = next;
    }
}

void DBList::addDBConnect(sql::Connection* con)
{
    auto* new_node = new DBListNode;
    new_node->last_ = new_node->next_ = nullptr;
    new_node->used_ = false;
    new_node->con_.reset(con);

    if (this->tail_ == nullptr){
        this->head_ = this->tail_ = new_node;
    } else{
        this->tail_->next_ = new_node;
        new_node->last_ = this->tail_;
        this->tail_ = new_node;
    }
}

DBListNode* DBList::useConnect()
{
    std::unique_lock<std::mutex> lock(this->mutex_);
    // 等待连接可用
    this->cv_.wait(lock, [this]() -> bool {
        return this->head_ && !this->head_->used_;
    });

    DBListNode* node = this->head_;
    // 将头节点放到最后
    if (node->next_){
        // 头节点为下一个节点
        node->next_->last_ = nullptr;
        this->head_ = node->next_;
        node->next_ = nullptr;
        // 放到尾节点
        node->last_ = this->tail_;
        this->tail_->next_ = node;
        this->tail_ = node;
    }
    lock.unlock();
    node->used_ = true;
    return node;
}

bool DBList::unUseConnect(DBListNode* con)
{
    if (!con || !con->used_){
        return false;
    }
    std::unique_lock<std::mutex> lock(this->mutex_);

    DBListNode* node = con;
    if (node->last_ != nullptr){
        node->last_->next_ = node->next_;
        if (node->next_ != nullptr) {
            node->next_->last_ = node->last_;
        }

        node->last_ = nullptr;
        node->next_ = this->head_;
        this->head_->last_ = node;
        this->head_ = node;
    }
    node->used_ = false;
    lock.unlock();
    return true;
}

