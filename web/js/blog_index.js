
class UrlManager
{
    static {
        this.getNewBlogs = "/api/blog/articles";
        this.searchBlog = "/api/blog/search";
    }
}

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

        box.addEventListener("click", function (){
            $.ajax({
                url: "/api/blog/article?id=" + box.getAttribute("blog_id"),
                type: "GET",
                success: function (res){
                    $("#main_app").hide();
                    $("#blog_detail_box").append(MD.makeHtml(myDecodeString(res)));
                },
                error: function (){
                    alert("获取失败");
                }
            })
        })

        return box;
    }
}

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
                    })
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
