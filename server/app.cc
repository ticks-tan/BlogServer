/****
* @Copyright(C) 2022.
*
* @filename: app.cc
* @date: 2022/6/6
* @author: Ticks
* @description: 
*
*
****/

#include "blog_router.h"
#include "app.h"


BlogApp::BlogApp()
    : _error(false)
{
    loadDefaultConfig();
}

__attribute__((unused))
BlogApp::BlogApp(const std::string &config_file)
{
    this->_config.configFile = config_file;
    if (!loadConfig(config_file)){
        printf(">> check config error!\n");
        this->_error = true;
        return;
    }
    this->_error = false;
}

// 加载配置文件
bool BlogApp::loadConfig(const std::string &config_file)
{
    if (config_file.empty()){
        return false;
    }
    this->_config.configFile = config_file;
    return this->checkConfig();
}

// 初始化服务
void BlogApp::initServer()
{
    this->_server.service = &this->_router;
    // 初始化博客接口
    initBlogRouter();

    if (this->_config.httpVersion != 2){
        this->_server.http_version = 1;
    } else{
        this->_server.http_version = 2;
    }
    // https
    if (this->_config.enableSSL){
        if (this->_config.httpsPort > 0) {
            this->_server.https_port = this->_config.httpsPort;
        } else{
            this->_server.https_port = 443;
        }
    }
    if (this->_config.httpPort > 0) {
        this->_server.port = this->_config.httpPort;
    } else{
        this->_server.port = 80;
    }
    // web
    if (this->_config.enableWeb){
        if (!this->_config.webHome.empty()){
            this->_router.document_root = this->_config.webHome;
        } else{
            this->_router.document_root = "./";
        }
        if (!this->_config.webHomeFile.empty()){
            this->_router.home_page = this->_config.webHomeFile;
        } else{
            this->_router.home_page = "index.html";
        }
        if (!this->_config.webErrorFile.empty()){
            this->_router.error_page = this->_config.webErrorFile;
        } else{
            this->_router.error_page = "404.html";
        }
    }
    if (this->_config.execCount > 0){
        this->_server.worker_processes = this->_config.execCount;
    } else{
        this->_server.worker_processes = 0;
    }
    if (this->_config.threadCount > 0){
        this->_server.worker_threads = this->_config.threadCount;
    } else{
        this->_server.worker_threads = 4;
    }

    // 初始化全局线程池
    if (this->_config.threadCount > 0){
        hv::async::startup(this->_config.threadCount, this->_config.threadCount * 2);
    } else{
        hv::async::startup(std::thread::hardware_concurrency(), std::thread::hardware_concurrency() * 2);
    }
}

//  正式运行
bool BlogApp::run()
{
    initDBPool();
    if (this->_error){
        printf(">> run app error!\n");
        return false;
    }
    if (!this->checkFirstConfig()){
        std::cout << "初始化数据库配置失败！\n";
        return false;
    }
    initServer();
    hlog_set_level(LOG_LEVEL_WARN);
    http_server_run(&this->_server);
    return true;
}

// 检查配置文件
bool BlogApp::checkConfig()
{
    if (this->_config.configFile.empty()){
        this->_error = true;
        return false;
    }
    std::ifstream in(this->_config.configFile);
    if (!in){
        this->_error = true;
        return false;
    }

    std::unordered_map<std::string, std::string> maps;
    std::string configStr;
    std::string configStrBak;
    int key_start = 0, key_end = 0, value_start = 0, value_end, i;
    int status = 0; // 0 为 key_start状态，1 为 value_start状态

    while (!in.eof()){
        getline(in, configStrBak);
        configStr += configStrBak + '\n';
    }

    configStrBak.reserve(configStr.size());

    // 剔除空格和注释
    auto p = configStr.begin();
    for (; p != configStr.end(); ++p){
        if (*p == '#'){
            // 向后忽略一行
            while (p != configStr.end() && *p != '\n'){
                ++p;
            }
        }else if (*p != ' '){
            configStrBak.push_back(*p);
        }
    }

    // 核心处理逻辑
    p = configStrBak.begin();
    for (i = 0; i < configStrBak.length() && p != configStrBak.end(); ++i, ++p){
        if (*p == '='){
            // 遇到等于
            if (status == 0){
                // 上一个状态为处理 Key
                if (i != key_start) {
                    // 记录 Key 结束位置
                    key_end = i;
                    // 状态变为处理 Value
                    status = 1;
                    // 记录 Value 开始位置
                    value_start = i + 1;
                } else{
                    return false;
                }
            }
        }else if (*p == '\n'){
            // 上一个状态为 处理value
            if (status == 1){
                if (i != value_start) {
                    value_end = i;
                    maps[configStrBak.substr(key_start, key_end - key_start)]
                            = configStrBak.substr(value_start, value_end - value_start);
                    status = 0;
                    key_start = i + 1;
                } else{
                    if (value_start != 0) {
                        return false;
                    }
                }
            }else if (i == key_start){
                key_start = i + 1;
            }
        }
    }

    // 加载默认配置
    this->loadDefaultConfig();
    std::unordered_map<std::string, std::string>::iterator it;
    auto end = maps.end();

    if ((it = maps.find("execCount")) != end){
        this->_config.execCount = std::stoi(it->second);
    }
    if ((it = maps.find("threadCount")) != end){
        this->_config.threadCount = std::stoi(it->second);
    }
    if ((it = maps.find("httpPort")) != end){
        this->_config.httpPort = std::stoi(it->second);
    }
    if ((it = maps.find("httpVersion")) !=end){
        this->_config.httpVersion = std::stoi(it->second);
    }
    if ((it = maps.find("logFile")) != end){
        this->_config.logFile = it->second;
    }
    if ((it = maps.find("enableSSL")) != end){
        if (it->second == "true"){
            this->_config.enableSSL = true;
            if ((it = maps.find("httpsPort")) != end){
                this->_config.httpsPort = std::stoi(it->second);
            }
        }
    }
    if ((it = maps.find("enableWeb")) != end){
        if (it->second == "true"){
            this->_config.enableWeb = true;
            if ((it = maps.find("webHome")) != end){
                this->_config.webHome = it->second;
            }
            if ((it = maps.find("webHomeFile")) != end){
                this->_config.webHomeFile = it->second;
            }
            if ((it = maps.find("webErrorFile")) != end){
                this->_config.webErrorFile = it->second;
            }
        } else{
            this->_config.enableWeb = false;
        }
    }

    if ((it = maps.find("dbHost")) != end){
        this->_config.dbHost = it->second;
    }
    if ((it = maps.find("dbName")) != end){
        this->_config.dbName = it->second;
    }
    if ((it = maps.find("dbPort")) != end){
        this->_config.dbPort = std::stoi(it->second);
    }
    if ((it = maps.find("dbUser")) != end){
        this->_config.dbUser = it->second;
    }
    if ((it = maps.find("dbPassword")) != end){
        this->_config.dbPassword = it->second;
    }

    if ((it = maps.find("initConfigPort")) != end){
        this->_config.initConfigPort = std::stoi(it->second);
    }
    return true;
}

// 加载默认配置文件
void BlogApp::loadDefaultConfig()
{
    this->_config.execCount = 0;
    this->_config.threadCount = 2;
    this->_config.httpPort = 80;
    this->_config.enableSSL = false;
    this->_config.httpsPort = 443;
    this->_config.enableWeb = true;
    this->_config.httpVersion = 1;
    this->_config.webHome = "./";
    this->_config.webHomeFile = "index.html";
    this->_config.webErrorFile = "404.html";

    this->_config.dbHost = "127.0.0.1";
    this->_config.dbPort = 3306;
    this->_config.dbName = "dbName";
    this->_config.dbUser = "root";
    this->_config.dbPassword = "password";
    this->_config.initConfigPort = 8080;
}


/**
 * 初始化数据库连接池
 * @return
 */
bool BlogApp::initDBPool()
{
    sql::SQLString url = "tcp://" + this->_config.dbHost + ":" +
            std::to_string(this->_config.dbPort) + "/" + this->_config.dbName;

    sql::Properties properties({
        {"username", this->_config.dbUser},
        {"password", this->_config.dbPassword}
    });

    try {
        this->_db_pool = DBPool::newInstance(url, properties, 100);
    }catch (MariaDB_Exception& exp){
        std::cout << exp.what() << std::endl;
        this->_error = true;
        return false;
    }catch (sql::SQLException& exp){
        std::cout << exp.what() << std::endl;
        this->_error = true;
        return false;
    }
    if (this->_db_pool->count() > 0) {
        return true;
    }
    this->_error = true;
    return false;
}

// Router 配置
void BlogApp::initBlogRouter()
{
    // 拦截浏览器请求
    this->_router.Any("/blog_files", &interceptWebBlogFile);
    this->_router.Any("/blog_files/*", &interceptWebBlogFile);
    this->_router.Any("/first_configure", &interceptWebBlogFile);
    this->_router.Any("/first_configure/*", &interceptWebBlogFile);
    // 获取一页博客信息
    this->_router.GET("/api/blog/articles", std::bind(&getPageArticlesInfo, this, std::placeholders::_1));
    // 获取一篇文章
    this->_router.GET("/api/blog/article", std::bind(&getBlogArticleContent, this, std::placeholders::_1, std::placeholders::_2));
    // 查询博客
    this->_router.GET("/api/blog/search", std::bind(&searchBlogArticle, this, std::placeholders::_1));
    // 上传博客
    this->_router.POST("/api/blog/post/article", std::bind(&uploadBlogArticle, this, std::placeholders::_1));
}

bool BlogApp::checkFirstConfig()
{
    auto conn = this->getDBConnection();
    if (conn.get() == nullptr){
        return false;
    }
    try {
        std::string queryStr = "show tables;";
        auto stmt(conn->createStatement());
        auto result = stmt->executeQuery(queryStr.c_str());
        if (result->rowsCount() >= 2){
            return true;
        }else{
            std::cout << "数据库未初始化，启动初始化网站，监听地址：127.0.0.1:" << this->_config.initConfigPort << "\n";
            http_server_t config_server;
            config_server.http_version = 1;
            if (this->_config.enableSSL){
                config_server.https_port = this->_config.initConfigPort;
            } else{
                config_server.port = this->_config.initConfigPort;
            }
            config_server.worker_threads = 1;

            HttpService service1;
            config_server.service = &service1;
            service1.document_root = this->_config.webHome + "/first_configure";
            service1.home_page = "index.html";
            service1.error_page = "404.html";
            service1.POST("/api/blog/init/config", std::bind(&firstInitBlogConfig, this, &config_server, false, std::placeholders::_1));
            http_server_run(&config_server);
            std::cout << "初始化完成，正在启动网站\n";
            return true;
        }
    }catch (sql::SQLException& exp){
        std::cout << exp.what() << std::endl;
        return false;
    }
    return false;
}
