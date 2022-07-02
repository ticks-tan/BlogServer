// URL API管理，同意写在此处
class UrlManager
{
    static {
        this.getNewBlogs = "/api/blog/articles";
        this.searchBlog = "/api/blog/search";
    }
}
// MarkDown 文章转 Html 工具
MD = new showdown.Converter({
    // 开启表格
    tables: true,
    // 开启任务列表
    tasklists: true,
    // 链接默认新窗口打开
    openLinksInNewWindow: true
});

// 初始化分页和博客数据
function initBlogDateAndPaging() {
    // 初始化分页
    P.initMathod({
        params: {
            elemId: '#home_page_paging_navigation',
            total: '0',
            pageNum: '5'
        },
        requestFunction: function (){
            getBlogs(P, $("#home_page_blogs_box"));
        }
    });
    // 获取博客
    P.config.pageIndex = 1;
    getBlogs(P, $("#home_page_blogs_box"));
}

// 初始化详情页面
function initBlogDetailPage() {
    // 隐藏博客详情页面
    $("#blog_detail_page").hide();

    $("#blog_detail_delete_img").on("click" , function (){
        $("#blog_detail_box").empty();
        $("#blog_home_page").show();
        $("#blog_detail_page").hide();
    });
    $("#blog_detail_share_img img").on("click", function (){
        let ele = $("text_copy");
        ele.text(window.location.href).show();
        let copy = document.getElementById("text_copy");
        copy.select();
        document.execCommand("copy", false, null);
        ele.hide();
        alert("已复制: " + window.location.href);
    })
}

// 初始化搜索框
function initSearchInput() {
    let result_box = $("#home_page_search_result_box");
    result_box.hide();
    let search_input = $("#home_page_search_input");
    let last_val = search_input.val();
    let show_box = $("#home_page_show_box");
    let search_result_blogs = $("#home_page_search_blogs_box");

    function search(){
        let val = search_input.val();
        if (val.length === 0 && val !== last_val){
            // 值为空
            search_result_blogs.empty();
            result_box.hide();
            show_box.show();
        }else if (val !== last_val){
            // 搜索结果
            search_result_blogs.empty();
            show_box.hide();
            result_box.show();
            $.ajax({
                url: UrlManager.searchBlog + "?key=" + val,
                type: "GET",
                success: function (res){
                    if (res.code === 200){
                        let json = res["articles"];
                        if (json && json instanceof Array){
                            json.forEach(function (val, index,array){
                                let blogItem = new BlogItem(val.id, val.title, val.tags,
                                    new Date(parseInt(val["updateDate"]) * 1000).toLocaleString());
                                search_result_blogs.append(blogItem.createNode());
                            });
                            if (json.length === 0){
                                search_result_blogs.append(new BlogErrorItem("没有你想要的博客呢～").createNode());
                            }
                        }else {
                            search_result_blogs.append(new BlogErrorItem("没有你想要的博客呢～").createNode());
                        }
                    }else {
                        search_result_blogs.append(
                            new BlogErrorItem(res.message ? res.message : "搜索失败").createNode()
                        );
                    }
                },
                error: function (){
                    search_result_blogs.append(new BlogErrorItem("请求失败!").createNode());
                }
            })
        }
        last_val = val;
    }

    search_input.keypress(function (event){
        if (event.which === 13){
            search();
        }
    });
    search_input.on("blur", function (){
        search();
    })
}

function initBackToTop()
{
    $("#back_top_btn").on("click", function() {
        $('#blog_detail_page,#blog_home_page').animate({
                scrollTop: 0
            }, 500);
    });
}

// 文档就绪事件 -- 写初始化函数
$(document).ready(function (){
    // 初始化返回顶部
    initBackToTop();
    // 初始化搜索框
    initSearchInput();
    // 初始化博客详情页面
    initBlogDetailPage();
    // 初始化分页和数据
    initBlogDateAndPaging();
});

// 博客数据获取失败显示的文字
class BlogErrorItem
{
    constructor(msg) {
        this.msg = msg;
    }
    createNode(){
        let box = document.createElement("div");
        box.classList.add("blog_error_item");
        box.innerText = this.msg;
        return box;
    }
}

// 博客标题
class BlogTitle
{
    constructor(title) {
        this.title = title;
    }

    createNode(){
        let center = document.createElement("center");
        let b = document.createElement("b");
        b.innerText = this.title;
        center.append(b);
        return center;
    }

}

/*
<div className="blog_item">
    <h3 className="blog_title blog_item_top" title="Mysql简单使用">Mysql简单使用</h3>
    <div className="blog_item_bottom">
        <div className="blog_tag_div">
            <span className="blog_tag">Mysql</span>
            <span className="blog_tag">教程</span>
        </div>
        <div className="blog_date">2022-6-12 23:23:23</div>
    </div>
</div>
 */
// 博客项目数据
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
        box.setAttribute("blog_id", this.blogId);

        let h3 = document.createElement("h3");
        h3.classList.add("blog_title");
        h3.classList.add("blog_item_top");
        h3.title = this.blogTitle;
        h3.innerText = this.blogTitle;

        let div = document.createElement("div");
        div.classList.add("blog_item_bottom");
        let div1 = document.createElement("div");
        div1.classList.add("blog_tag_div");
        let tags = this.blogTags.split(',');
        for (let i = 0; i < tags.length; ++i){
            let span = document.createElement("span");
            span.classList.add("blog_tag");
            span.innerText = tags.at(i);
            div1.append(span);
        }

        let date = document.createElement("div");
        date.classList.add("blog_date");
        date.innerText = this.blogDate;

        div.append(div1);
        div.append(date);
        box.append(h3);
        box.append(div);

        // 文章点击时会执行的函数
        box.addEventListener("click", function (){
            // 显示博客详情页面，再加载数据，防止多次点击主页面请求多个数据。
            $("#blog_home_page").hide();
            $("#blog_detail_page").show();
            let detail_box = $("#blog_detail_box");
            // 设置文章标题
            detail_box.append(new BlogTitle(h3.title).createNode());
            detail_box.append("<hr>")
            // Ajax异步获取数据
            $.ajax({
                url: "/api/blog/article?id=" + box.getAttribute("blog_id"),
                type: "GET",
                success: function (res){
                    // 添加数据到容器并渲染
                    detail_box.append(MD.makeHtml(myDecodeString(res)));
                    // 高亮代码块
                    hljs.highlightAll();
                },
                error: function (){
                    // 加载失败，显示加载失败数据
                    let p = document.createElement("p");
                    p.innerText = "博客数据加载失败！";
                    detail_box.append(p);
                }
            })
        });

        return box;
    }
}

// 获取博客列表
function getBlogs(pageObj, blogBox)
{
    $.ajax({
        url: UrlManager.getNewBlogs + "?page=" + pageObj.config.pageIndex,
        type: 'GET',
        dataType: 'json',
        success: function(res){
            blogBox.empty();
            if (parseInt(res.code) === 200){
                pageObj.config.total = parseInt(res.count);
                let json = res["articles"];
                if (json && json instanceof Array){
                    json.forEach(function (val, index,array){
                        let blogItem = new BlogItem(val.id, val.title, val.tags,
                            new Date(parseInt(val["updateDate"]) * 1000).toLocaleString());
                        blogBox.append(blogItem.createNode());
                    });
                }
            }else {
                // 添加错误信息
                blogBox.append(new BlogErrorItem(res["message"]).createNode());
            }
            pageObj.pageHtml();
        },
        error: function(){
            console.log("获取失败");
            blogBox.empty();
            // 添加错误信息
            blogBox.append(new BlogErrorItem("获取信息失败").createNode());
            pageObj.pageHtml();
        }
    });
}

ChMap = [
    ['0', 'A'], ['1', 'B'],
    ['2', 'C'], ['3', 'D'],
    ['4', 'E'], ['5', 'F'],
    ['6', 'G'], ['7', 'H'],
    ['8', 'I'], ['9', 'J']
];
IndexMap = [
    ['0', 'K'], ['1', 'L'],
    ['2', 'M'], ['3', 'N'],
    ['4', 'O'], ['5', 'P'],
    ['6', 'Q'], ['7', 'R'],
    ['8', 'S'], ['9', 'T']
];
// 自定义编码函数
function myDecodeString(data)
{
    let tmp = "", num = "", decode = "";
    let index = 0;
    let last = 0, it = data.indexOf(IndexMap[index][1]);
    while (it !== -1){
        tmp = data.slice(last, it);
        for (let i = 0; i < tmp.length; ++i){
            num += ChMap[tmp.charCodeAt(i) - 'A'.charCodeAt(0)][0];
        }
        decode += String.fromCharCode(parseInt(num));
        num = "";
        last = it + 1;
        index = (index + 1) % 10;
        it = data.indexOf(IndexMap[index][1], last);
    }
    return decode.toString();
}
