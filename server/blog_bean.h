/**
* @Copyright(C) 2022.
*
* @filename: blog_bean.h
* @date: 22-6-13
* @author: Ticks
* @description: 
*
* If you have contact or bugs, you can email to me, I can help you.
**/

#ifndef __BLOG_SERVER_BLOG_BEAN_H__
#define __BLOG_SERVER_BLOG_BEAN_H__

#include <string>

// 博客文章
struct BlogArticle
{
    // 文章ID
    std::string id;
    // 博客标题
    std::string title;
    // 博客作者
    std::string author;
    // 博客标签
    std::string tags;
    // 博客上传时间
    long uploadDate;
    // 博客最后更新时间
    long updateDate;
};

// 博客用户
struct BlogAuthor
{
    // 用户名
    std::string userName;
    // 昵称
    std::string nickName;
    // 密码
    std::string password;
    // 密码md5签名
    std::string mdPassword;
    // 邮箱
    std::string email;
    // 签名
    std::string note;
};

#endif // __BLOG_SERVER_BLOG_BEAN_H__
