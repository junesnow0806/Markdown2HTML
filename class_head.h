#ifndef MDtoHTML
#define MDtoHTML

#include <cstdlib>
#include <fstream>
#include <vector>
#include <cstring>
#include <utility>
#include <string>
#include <cctype>
#include <cstdio>
using namespace std;

#define MAX_LINE_LEN 10000

//markdown语法关键词枚举
enum
{
    nul = 0,
    paragraph = 1,
    href = 2,
    ul = 3,
    ol = 4,
    li = 5,
    em = 6,
    strong = 7,
    hr = 8,
    br = 9,
    image = 10,
    quote = 11,
    h1 = 12,
    h2 = 13,
    h3 = 14,
    h4 = 15,
    h5 = 16,
    h6 = 17,
    blockcode = 18,
    code = 19
};
//HTML前置标签
const string frontTag[] = {
    "", "<p>", "", "<ul>", "<ol>", "<li>", "<em>", "<strong>",
    "<hr color=#CCCCCC size=1 />", "<br />",
    "", "<blockquote>",
    "<h1 ", "<h2 ", "<h3 ", "<h4 ", "<h5 ", "<h6 ", //右边的括号预留给添加其他的标签属性
    "<pre><code>", "<code>"};
//HTML后置标签
const string backTag[] = {
    "", "</p>", "", "</ul>", "</ol>", "</li>", "</em>", "</strong>",
    "", "", "", "</blockquote>",
    "</h1>", "</h2>", "</h3>", "</h4>", "</h5>", "</h6>",
    "</code></pre>", "</code>"};
typedef struct Cnode
{
    vector<Cnode *> ch;
    string heading;
    string tag;
    Cnode(const string &hd) : heading(hd) {} //TODO:
} Cnode;

typedef struct node
{
    int type; //结点代表的md词法类型
    vector<node *> ch;
    string elem[3];                  //存放三个重要属性, elem[0]存放要显示的内容
                                     //elem[1]存放链接, elem[2]存放title
    node(int _type) : type(_type) {} //TODO:
} node;                              //语法树的结点, 一个结点存放一行的信息

class Derivier
{
private:
    node *root, *now;
    Cnode *Croot;
    string content, TOC;
    int cntTag = 0;
    char s[MAX_LINE_LEN];

    inline bool isHeading(node *v);
    inline bool isImage(node *v);
    inline bool isHref(node *v);
    template <typename T>
    void destroy(T *v);
    void Cdfs(Cnode *v, string index);
    void Cins(Cnode *v, int x, const string &hd, int tag);
    void dfs(node *v);
    inline bool isCutline(char *src);
    inline void mkpara(node *v);
    inline pair<int, char *> start(char *src);
    inline pair<int, char *> JudgeType(char *src);
    inline node *findnode(int depth);
    void insert(node *v, const string &src);

public:
    Derivier(const std::string &filename);
    string getTableOfContents();
    string getContents();
    ~Derivier();
};

#endif
