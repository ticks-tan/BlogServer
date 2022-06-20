/*
<div class="blog_item">
    <h3 class="blog_title" title="Mysql简单使用">Mysql简单使用</h3>
    <h5 class="blog_date">2022-6-12 23:23:23</h5>
</div>
 */

class UrlManager
{
    static {
        this.getNewBlogs = "/api/blog/articles";
        this.searchBlog = "/api/blog/search";
    }
}

function toastMessage(msg){
    let toast = document.getElementById("toast_box");
    toast.innerText = msg;
    toast.style.animation = "toastAnimation 4s linear";
    toast.addEventListener("animationend", function (){
        toast.style.animation = "";
    });
}

// 加载更多
class MyLoadMore{
    constructor(load_more) {
        this.run = false;
        this.loadMore = load_more;
        this.timeId = 0;
        this.letters = ['⬤','⬤','⬤','⬤','⬤','⬤','⬤'];
    }

    // 开始加载
    startLoadMore(){
        this.run = true;
        this.start();
    }
    // 开始动画
    start(){
        let this_ = this;
        this.loadMore.innerHTML = "";
        for (let i = 0; i < this.letters.length; ++i) {
            let span = document.createElement("span");
            span.textContent = this.letters[i];
            span.style.animationDelay = i * 0.1 + "s";
            this.loadMore.append(span);
        }
        if (this_.run === true) {
            this_.timeId = setTimeout(function () {
                this_.start();
            }, 2000);
        }
    }
    // 停止加载
    stopLoadMore(){
        this.run = false;
        clearTimeout(this.timeId);
        this.loadMore.innerHTML = "点击加载更多";
    }
}

class BlogItem
{
    constructor(id, title, tags, date) {
        // 博客ID
        this.blogId = id;
        // 博客标题
        this.blogTitle = title;
        // 博客最新时间
        this.blogDate = date;
        // 标签
        this.blogTags = tags;
    }

    // 创建节点
    createNode()
    {
        let box = document.createElement("div");
        box.classList.add("blog_item");
        box.blog_id = this.blogId;

        let h3 = document.createElement("h3");
        h3.classList.add("blog_title");
        h3.title = this.blogTitle;
        h3.innerText = this.blogTitle;

        let h5 = document.createElement("h5");
        h5.classList.add("blog_date");
        h5.innerText = this.blogDate;

        box.append(h3);
        box.append(h5);

        return box;
    }
}

// Http请求客户端
class HttpClient
{
    constructor() {
    }

    GET(url, okCall, errorCall = null){
        const http = new XMLHttpRequest();
        http.open("GET", url);
        http.send();
        http.onreadystatechange = function(){
            if (this.readyState === 4){
                if (this.status === 200 && okCall){
                    okCall(http.responseText);
                }else {
                    if (errorCall){
                        errorCall();
                    }
                }
            }
        }
    }
}

class Blogs
{
    constructor(boxId) {
        this.blogBox = document.getElementById(boxId);
        this.page = 1;
        this.blogArray = [];
        this.searchBlogArray = [];
    }

    // 获取博客
    getNewBlog(loadMore) {
        let httpClient = new HttpClient();
        loadMore.startLoadMore();
        let this_ = this;
        httpClient.GET( UrlManager.getNewBlogs + "?page=" + this.page, function (content){
            let json = JSON.parse(content);
            if (json){
                if (json["code"] === 200){
                    // 成功
                    let blogs = json["articles"];
                    if (blogs && blogs.isArray){
                        for (let i = 0; i < blogs.length; ++i){
                            let blog = blogs[i];
                            if (blog){
                                this_.blogArray.append(
                                    new BlogItem(blog["id"], blog["title"], blog["tags"], blog["updateDate"])
                                );
                            }
                        }
                    }
                }
            }
            ++this_.page;
            loadMore.stopLoadMore();
            // 更新文章

        }, function (){
            toastMessage("获取博客失败");
            loadMore.stopLoadMore();
        });
    }


    // 搜索博客
    searchBlog(searchInput) {
        let httpClient = new HttpClient();
        let this_ = this;

        httpClient.GET(UrlManager.searchBlog + "?key=" + searchInput.value, function (content) {
            this_.searchBlogArray = [];
            let json = JSON.parse(content);
            if (json){
                if (json["code"] === 200){
                    let blogs = json["articles"];
                    if (blogs && blogs.isArray){
                        for (let i = 0; i < blogs.length; ++i){
                            let blog = blogs[i];
                            if (blog){
                                this_.searchBlogArray.append(
                                    new BlogItem(blog["id"], blog["title"], blog["tags"], blog["update_date"])
                                );
                            }
                        }
                    }
                }
            }
            // 显示文章
        }, function () {
            // 请求出错
        });
    }
}