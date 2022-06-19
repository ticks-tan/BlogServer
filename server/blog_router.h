/**
* @Copyright(C) 2022.
*
* @filename: blog_router.h
* @date: 22-6-13
* @author: Ticks
* @description: 博客相关接口
*
* If you have contact or bugs, you can email to me, I can help you.
**/

#ifndef __BLOG_SERVER_BLOG_ROUTER_H__
#define __BLOG_SERVER_BLOG_ROUTER_H__

#include <locale>
#include <codecvt>
#include "blog_bean.h"
#include "hv/HttpService.h"
#include "hv/md5.h"
#include "app.h"

static char ChMap[][2] = {
        {'0', 'A'}, {'1', 'B'},
        {'2', 'C'}, {'3', 'D'},
        {'4', 'E'}, {'5', 'F'},
        {'6', 'G'}, {'7', 'H'},
        {'8', 'I'}, {'9', 'J'}
};

static char IndexMap[][2] = {
        {'0', 'K'}, {'1', 'L'},
        {'2', 'M'}, {'3', 'N'},
        {'4', 'O'}, {'5', 'P'},
        {'6', 'Q'}, {'7', 'R'},
        {'8', 'S'}, {'9', 'T'}
};

static std::wstring toWideString(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    return convert.from_bytes(str);
}

static std::string toByteString(const std::wstring str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    return convert.to_bytes(str);
}

static std::string myEncodeString(const std::string& data)
{
    std::string encode, tmp;
    std::wstring wdata = toWideString(data);
    size_t index = 0;
    for (auto it : wdata){
        tmp = std::to_string((unsigned long)it);
        for (auto ch : tmp){
            encode.push_back(ChMap[ch - '0'][1]);
        }
        encode.push_back(IndexMap[index][1]);
        index = (index + 1) % 10;
    }
    return encode;
}

static std::string myDecodeString(const std::string data)
{
    std::string tmp, num;
    std::wstring decode;
    size_t index = 0;
    size_t last = 0, it = data.find_first_of(IndexMap[index][1]);
    while (it != std::string::npos){
        tmp = data.substr(last, it - last);
        for (auto ch : tmp){
            num.push_back(ChMap[ch - 'A'][0]);
        }
        decode.push_back(wchar_t(std::stol(num)));
        num.clear();
        last = it + 1;
        index = (index + 1) % 10;
        it = data.find_first_of(IndexMap[index][1], last);
    }
    return toByteString(decode);
}


// 拦截web访问博客文章目录
static int interceptWebBlogFile(HttpRequest* req, HttpResponse* resp)
{
    // resp->SetHeader("Content-Type", "text/html;charset=utf8");
    // resp->SetBody("<p1 style=\"text-align:center;\">Request Not Allow!</p1><hr>");
    return HTTP_STATUS_NOT_FOUND;
}


// 获取指定篇数的博客, 数据库查询操作，比较耗时
static int getPageArticlesInfo(BlogApp* app, const HttpContextPtr& ptr)
{
    hv::Json json;

    // 设置回应报文类型
    ptr->setContentType("application/json;charset=utf8");
    // 解析 URl
    ptr->request->ParseUrl();

    std::string pageStr = ptr->request->GetParam("page", "");
    if (pageStr.empty()){
        // 请求参数无效
        json["code"] = 500;
        json["message"] = "request param loss";
        return ptr->sendJson(json);
    }
    int page = std::stoi(pageStr);

    if (page > 0){
        try{
            // 获取数据库连接
            auto con = app->getDBConnection();
            if (con.get() == nullptr){
                json["code"] = 500;
                json["message"] = "server database not found";
                return ptr->sendJson(json);
            }
            pageStr = "select * from blogs order by update_date desc limit "
                    + std::to_string(15 * (page - 1)) + "," + std::to_string(15 * page);
            auto statement = con->createStatement();
            auto result(statement->executeQuery(sql::SQLString(pageStr.data())));

            json["code"] = 200;
            hv::Json article;
            // 遍历结果
            while (result->next()){
                article["id"] = result->getString(1).c_str();
                article["title"] = result->getString(2).c_str();
                article["author"] = result->getString(3).c_str();
                article["tags"] = result->getString(4).c_str();
                article["upload_date"] = result->getLong(5);
                article["update_date"] = result->getLong(6);
                json["articles"].push_back(article);
            }
            return ptr->sendJson(json);
        }catch (sql::SQLException& exp){
            // 日至记录
            LOGW("%s", exp.what());
            json["code"] = 500;
            json["message"] = "server database error";
            return ptr->sendJson(json);
        }catch (hv::Json::exception& exp){
            LOGW("%s", exp.what());
            return HTTP_STATUS_INTERNAL_SERVER_ERROR;
        }
    }
    json["code"] = 500;
    json["message"] = "request param error";
    return ptr->sendJson(json);
}


// 获取单篇博客，文件操作和编码，慢
static int getBlogArticleContent(BlogApp* app, const HttpRequestPtr& req, const HttpResponseWriterPtr& resp)
{
    // 开始
    resp->Begin();
    // 解析参数
    req->ParseUrl();
    std::string id_str = req->GetParam("id", "");
    if (id_str.empty()){
        resp->WriteStatus(HTTP_STATUS_NOT_FOUND);
        resp->WriteHeader("Content-Type", "text/plain;charset=utf8");
        resp->WriteBody("Sorry, I can't find the article!");
        resp->End();
        return HTTP_STATUS_UNFINISHED;
    }
    // 获取文件路径
    std::string file_path = app->getConfig().webHome + "/blog_files/" + id_str + ".md";
    HFile file;
    // 打开文件
    if (file.open(file_path.c_str(), "r") != 0){
        resp->WriteStatus(HTTP_STATUS_NOT_FOUND);
        resp->WriteHeader("Content-Type", "text/plain;charset=utf8");
        resp->WriteBody("Sorry, I can't find the article!");
        resp->End();
        return HTTP_STATUS_UNFINISHED;
    }
    std::string file_content;
    // 读取文件并编码
    file_content.resize(file.size());
    file.readall(file_content);
    std::string encode_file_content = myEncodeString(file_content);
    if (encode_file_content.empty()){
        resp->WriteStatus(HTTP_STATUS_NOT_FOUND);
        resp->WriteHeader("Content-Type", "text/plain;charset=utf8");
        resp->WriteBody("Sorry, I can't find the article!");
        resp->End();
        return HTTP_STATUS_UNFINISHED;
    }
    resp->WriteStatus(HTTP_STATUS_OK);
    resp->WriteHeader("Content-Type", "text/plain;charset=utf8");
    resp->WriteBody(encode_file_content);
    resp->End();
    return HTTP_STATUS_UNFINISHED;
}


// 搜索博客，暂不支持多关键词搜索～
static int searchBlogArticle(BlogApp* app, const HttpContextPtr& ptr)
{
    hv::Json json;
    if (ptr->request->method != HTTP_GET){
        json["code"] = 500;
        json["message"] = "request method error";
        return ptr->sendJson(json);
    }

    ptr->request->ParseUrl();
    std::string key = ptr->param("key", "");
    if (!key.empty()){
        try{
            auto conn = app->getDBConnection();
            if (conn.get() == nullptr){
                json["code"] = 500;
                json["message"] = "server database not found";
                return ptr->sendJson(json);
            }
            std::string queryStr = "select * from blogs where title like '%"+ key +"%' or find_in_set(?,tags) order by update_date desc";
            auto stmnt(conn->prepareStatement(queryStr.c_str()));
            stmnt->setString(1, key);
            auto result = stmnt->executeQuery();

            json["code"] = 200;
            hv::Json article;
            while (result->next()){
                article["id"] = result->getString(1).c_str();
                article["title"] = result->getString(2).c_str();
                article["author"] = result->getString(3).c_str();
                article["tags"] = result->getString(4).c_str();
                article["upload_date"] = result->getLong(5);
                article["update_date"] = result->getLong(6);
                json["articles"].push_back(article);
            }
            return ptr->sendJson(json);
        }catch (sql::SQLException& exp){
            LOGW("%s", exp.what());
            json["code"] = 500;
            json["message"] = "server database error";
            return ptr->sendJson(json);
        }catch (hv::Json::exception& exp){
            LOGW("%s", exp.what());
            return HTTP_STATUS_INTERNAL_SERVER_ERROR;
        }
    }
    json["code"] = 500;
    json["message"] = "request param loss";
    return ptr->sendJson(json);
}


// 上传博客
static int uploadBlogArticle(BlogApp* app, const HttpContextPtr& ptr)
{
    hv::Json json;
    if (!ptr->is(MULTIPART_FORM_DATA)){
        json["code"] = 500;
        json["message"] = "request method error";
        return ptr->sendJson(json);
    }
    auto ret = hv::async([&json, ptr, app]() -> int{
        ptr->writer->Begin();
        auto& form = ptr->request->GetForm();

        // 文章标题
        auto iter = form.find("title");
        std::string title = (iter != form.end()) ? iter->second.content : "";

        // 文章标签
        iter = form.find("tags");
        std::string tags = (iter != form.end()) ? iter->second.content : "";

        // 用户名
        iter = form.find("u");
        std::string user = (iter != form.end()) ? iter->second.content : "";

        // 密码
        iter = form.find("p");
        std::string passwd = (iter != form.end()) ? iter->second.content : "";

        // 博客文件
        iter = form.find("article");
        // 判断参数
        if (title.empty() || tags.empty() || user.empty() || passwd.empty() || iter == form.end()){
            json["code"] = 500;
            json["message"] = "request param error";
            ptr->writer->WriteStatus(HTTP_STATUS_OK);
            ptr->writer->WriteHeader("Content-Type", "application/json;charset=utf8");
            ptr->writer->WriteBody(json.dump(4));
            ptr->writer->End();
            return HTTP_STATUS_UNFINISHED;
        }
        auto& article = iter->second;
        try{
            auto conn = app->getDBConnection();
            if (conn.get() == nullptr){
                json["code"] = 500;
                json["message"] = "server database not found";
                ptr->writer->WriteStatus(HTTP_STATUS_OK);
                ptr->writer->WriteHeader("Content-Type", "application/json;charset=utf8");
                ptr->writer->WriteBody(json.dump(4));
                ptr->writer->End();
                return HTTP_STATUS_UNFINISHED;
            }
            std::string tmpStr = "select * from user where username=? and md5_password=? limit 0,1";
            auto preStmt(conn->prepareStatement(tmpStr.c_str()));
            preStmt->setString(1, user);
            preStmt->setString(2, passwd);
            auto result = preStmt->executeQuery();
            if (result->getRow() == 0){
                // 用户未找到
                json["code"] = 500;
                json["message"] = "username or password error";
                ptr->writer->WriteStatus(HTTP_STATUS_OK);
                ptr->writer->WriteHeader("Content-Type", "application/json;charset=utf8");
                ptr->writer->WriteBody(json.dump(4));
                ptr->writer->End();
                return HTTP_STATUS_UNFINISHED;
            }
            // 插入文章
            long now_time = std::time(nullptr);
            std::string id = title + std::to_string(now_time);
            unsigned char id_[16];
            hv_md5((unsigned char*)id.data(), id.size(), id_);
            id = (char*)id_;

            tmpStr = "insert into blogs (id, title, author, tags, upload_date, update_date) values (?,?,?,?,?,?)";
            auto preStmt1(conn->prepareStatement(tmpStr.c_str()));
            preStmt1->setString(1, id);
            preStmt1->setString(2, title);
            preStmt1->setString(3, result->getString("nickname"));
            preStmt1->setString(4, tags);
            preStmt1->setLong(5, now_time);
            preStmt1->setLong(6, now_time);

            if (preStmt1->executeUpdate() == 1){
                json["code"] = 200;
                json["message"] = "article upload success";
                ptr->writer->WriteStatus(HTTP_STATUS_OK);
                ptr->writer->WriteHeader("Content-Type", "application/json;charset=utf8");
                ptr->writer->WriteBody(json.dump(4));
                ptr->writer->End();
                return HTTP_STATUS_UNFINISHED;
            }
            json["code"] = 500;
            json["message"] = "article upload error";
            ptr->writer->WriteStatus(HTTP_STATUS_OK);
            ptr->writer->WriteHeader("Content-Type", "application/json;charset=utf8");
            ptr->writer->WriteBody(json.dump(4));
            ptr->writer->End();
            return HTTP_STATUS_UNFINISHED;
        }catch (sql::SQLException& exp){
            json["code"] = 500;
            json["message"] = "server database error";
            ptr->writer->WriteStatus(HTTP_STATUS_OK);
            ptr->writer->WriteHeader("Content-Type", "application/json;charset=utf8");
            ptr->writer->WriteBody(json.dump(4));
            ptr->writer->End();
            return HTTP_STATUS_UNFINISHED;
        }
    });
    return ret.get();
}

#endif // __BLOG_SERVER_BLOG_ROUTER_H__
