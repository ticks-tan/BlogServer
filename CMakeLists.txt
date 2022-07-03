cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 11)

project(BlogServer)
# 项目代码
list(APPEND SERVER_SOURCE
        "server/main.cc"
        "server/app.h"
        "server/app.cc"
        "server/db_mariadb.h"
        "server/db_list.h"
        "server/db_list.cc"
        "server/db_mariadb.cc"
        "server/blog_router.h"
        "server/blog_bean.h"
        )

add_executable(BlogServer ${SERVER_SOURCE})

target_include_directories(BlogServer
        PRIVATE
        ${PROJECT_SOURCE_DIR}/server/include)

# 链接 openssl
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    message(STATUS "OpenSSL Found!")
endif()
target_link_libraries(BlogServer OpenSSL::Crypto OpenSSL::SSL)

# 链接 libhv nghttp2 mariadb
target_link_libraries(BlogServer
        libhv_static.a
        libnghttp2
        mariadbcpp)
