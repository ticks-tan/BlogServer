/**
* @Copyright(C) 2022.
*
* @filename: app.h
* @date: 2022/6/6
* @author: Ticks
* @description: App
*
* If you have contact or bugs, you can email to me, I can help you.
**/

#ifndef __BLOG_SERVER_APP_H__
#define __BLOG_SERVER_APP_H__

#include <fstream>
#include "hv/hlog.h"
#include "hv/HttpServer.h"
#include "hv/hasync.h"
#include "db_mariadb.h"

// App设置
struct AppConfig
{
    // 配置文件路径
    std::string configFile;
    // http版本
    int httpVersion;
    // http端口
    int httpPort;
    // 是否启用https
    bool enableSSL;
    // https端口
    int httpsPort;
    // 线程数量
    int threadCount;
    // 进程数量
    int execCount;
    // 日志文件
    std::string logFile;
    // 是否开启静态文件托管
    bool enableWeb;
    // 静态文件根目录
    std::string webHome;
    // 静态文件主文件
    std::string webHomeFile;
    // 404文件
    std::string webErrorFile;
    // 博客文件存放目录
    std::string blogFileHome;

    // 数据库Host
    std::string dbHost;
    // 数据库端口
    int dbPort;
    // 数据库名称
    std::string dbName;
    // 数据库用户名
    std::string dbUser;
    // 数据库用户密码
    std::string dbPassword;
    // 外部初始化端口
    int initConfigPort;
};

class BlogApp final
{
public:
    // 构造函数
    BlogApp();
    ~BlogApp()
    {
        hv::async::cleanup();
    }

    __attribute__((unused)) explicit BlogApp(const std::string& config_file);
    // 加载配置文件
    bool loadConfig(const std::string& config_file);
    // 开始运行
    bool run();

    // 获取服务
    HttpService& service()
    {
        return this->_router;
    }

    // 获取数据库连接
    DBPool::DBConnection getDBConnection(){
        return this->_db_pool->getConnection();
    }

    AppConfig& getConfig()
    {
        return this->_config;
    }

private:
    // 初始化服务
    void initServer();
    // 初始化数据库连接池
    bool initDBPool();
    // 检查配置文件
    bool checkConfig();
    // 加载默认配置
    void loadDefaultConfig();
    // 初始化 Blog Router
    void initBlogRouter();
    // 第一次初始化
    bool checkFirstConfig();
private:
    // http服务
    http_server_t _server;
    // router
    HttpService _router;
    // 配置文件
    AppConfig _config;
    // 是否有错误
    bool _error;
    // 数据库连接池
    std::shared_ptr<DBPool> _db_pool;
};


#endif // __BLOG_SERVER_APP_H__
