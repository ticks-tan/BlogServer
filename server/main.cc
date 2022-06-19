/**
* @Copyright(C) 2022.
*
* @filename: main.cc
* @date: 2022/6/6
* @author: Ticks
* @description: 
*
* If you have contact or bugs, you can email to me, I can help you.
**/

// #include "app.h"
// #include "db_mariadb.h"
#include "blog_router.h"

int main()
{
    BlogApp app;
    // 加载配置文件
    if (!app.loadConfig("./ServerConfig.txt")){
        std::cout << "读取配置错误！\n";
        return 1;
    }
    app.run();
    return 0;
}
