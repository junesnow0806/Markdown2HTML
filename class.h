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
#include <stdlib.h>
#include <iostream>
using namespace std;

#define MAX_LINE_LEN 10000

//markdown语法关键词枚举
//FIXME:增加下划线
enum
{
    nul = 0,            //  开始
    paragraph = 1,      //  段落
    href = 2,           //  超链接
    ul = 3,             //  无序列表
    ol = 4,             //  有序列表
    li = 5,             //  列表
    em = 6,             //  斜体
    strong = 7,         //  加粗
    hr = 8,             //  水平分割线
    br = 9,             //  换行
    image = 10,         //  图片
    quote = 11,         //  引用
    h1 = 12,            //  h1
    h2 = 13,            //  h2
    h3 = 14,            //  h3
    h4 = 15,            //  h4
    h5 = 16,            //  h5
    h6 = 17,            //  h6
    blockcode = 18,     //  代码段
    code = 19,          //  行内代码
    del = 20,           //  删除线
    underline = 21      //  下划线
};
//HTML前置标签
const string frontTag[] = {
    "", "<p>", "", "<ul>", "<ol>", "<li>", "<em>", "<strong>",
    "<hr color=#CCCCCC size=1 />", "<br />",
    "", "<blockquote>",
    "<h1 ", "<h2 ", "<h3 ", "<h4 ", "<h5 ", "<h6 ", //右边的括号预留给添加其他的标签属性
    "<pre><code>", "<code>", "<del>", "<u>"};
//HTML后置标签
const string backTag[] = {
    "", "</p>", "", "</ul>", "</ol>", "</li>", "</em>", "</strong>",
    "", "", "", "</blockquote>",
    "</h1>", "</h2>", "</h3>", "</h4>", "</h5>", "</h6>",
    "</code></pre>", "</code>", "</del>", "</u>"};

//存放目录的结构, 一个结点即一个目录项, 以下为一个目录项所含内容
typedef struct Tnode
{
    vector<Tnode *> vec;
    string head; //标题内容
    string tag;
    Tnode(const string &headstr) : head(headstr) {} //TODO:
} Tnode;

//存放正文的结构
typedef struct Cnode
{
    int type; //结点代表的md词法类型
    vector<Cnode *> vec;
    string elem[3];                   //存放三个重要属性, elem[0]存放要显示的内容
                                      //elem[1]存放链接, elem[2]存放title
    Cnode(int _type) : type(_type) {} //TODO:
} Cnode;                              //语法树的结点, 一个结点存放一行的信息

class Deriver
{
private:
    Cnode *Croot, *current;
    Tnode *Troot;
    string content, TOC;
    int cntTag = 0;          //为了让目录能够正确地索引到HTML中对应内容的位置而引入的记录
    char line[MAX_LINE_LEN]; //缓存要处理的行

    //判断是否为标题
    inline bool isHeading(Cnode *v)
    {
        return (v->type >= h1 && v->type <= h6);
    }
    //判断是否为图片
    inline bool isImage(Cnode *v)
    {
        return (v->type == image);
    }
    //判断是否为超链接
    inline bool isHref(Cnode *v)
    {
        return (v->type == href);
    }
    //递归销毁树的结点
    template <typename T>
    void destroy(T *v);

    //目录语法树的深度优先遍历
    void Tdfs(Tnode *v, string index);

    /*FIXME:
     *正文语法树的深度优先遍历
     *返回当前层的结点类型，比如del、strong、em等
     */
    int Cdfs(Cnode *v);
    /*判断是否是分割线
     *分割线的markdown语有以下三种模式
     * "***"或"---"
     * "* * *"或"- - -"
     * "*****"或"----------"
     * FIXME:注意！！！如果"***"后面跟了文本，这一行就不是分割线了！！！
     */
    inline bool isCutline(char *src);

    //生成段落
    inline void makePara(Cnode *v);

    /*行首的空格和tab不影响对markdown词法的解析
     *所以要先处理行首的空格和tab
     *src：要处理的一行字符串
     *返回: 由空格数和正文开始处的char*指针组成的pair类型
     */
    inline pair<int, char *> tabcount(char *src);

    /*判断当前行的类型
     *src: 所要判断的行
     *返回: 当前行的类型和除去行标志性关键字的正式内容的char*组成的pair类型
     */
    inline pair<int, char *> LineType(char *src);

    //给定树的深度depth, 寻找结点, 返回结点指针
    inline Cnode *findnode(int depth);

    /*新增结点的插入
     *结点的插入分两种类型
     *一种是向Tnode结构的树中插入目录结点
     *设置tag标签来标记一个目录结点所指向的正文结点, 
     *以实现点击目录项即可正确跳转到正文对应的位置
     */
    void Tinsert(Tnode *v, int x, const string &headstr, int tag);

    //向指定的结点v中插入要处理的串src
    //TODO:
    void Cinsert(Cnode *v, const string &src);

public:
    //构造函数
    Deriver(const std::string &filename);

    //获取markdown目录
    string getTable() { return TOC; }
    //获取markdown内容
    string getContent() { return content; }

    //析构函数
    ~Deriver()
    {
        destroy<Cnode>(Croot);
        destroy<Tnode>(Troot);
    }
};

//类函数定义
    template <typename T>
    void Deriver::destroy(T *v)
    {
        for (int i = 0; i < (int)v->vec.size(); i++)
        {
            destroy(v->vec[i]);
        }
        delete v;
    }

    void Deriver::Tdfs(Tnode *v, string index)
    {
        TOC += "<li>\n";
        TOC += "<a href=\"#" + v->tag + "\">" + index + " " + v->head + "</a>\n";
        int n = (int)v->vec.size();
        if (n)
        {
            TOC += "<ul>\n";
            for (int i = 0; i < n; i++)
            {
                Tdfs(v->vec[i], index + to_string(i + 1) + ".");
            }
            TOC += "</ul>\n";
        }
        TOC += "</li>\n";
    }

    int Deriver::Cdfs(Cnode *v)
    {
        if (v->type == paragraph && v->elem[0].empty() && v->vec.empty())
            return 0;

        content += frontTag[v->type];
        bool flag = true;

        //处理标题, 支持用目录进行跳转
        if (isHeading(v))
        {
            content += "id=\"" + v->elem[0] + "\">";
            flag = false;
        }

        //处理超链接
        if (isHref(v))
        {
            content += "<a href=\"" + v->elem[1] + "\" title=\"" + v->elem[2] + "\">" + v->elem[0] + "</a>";
            flag = false;
        }

        //处理图片
        if (isImage(v))
        {
            content += "<img alt=\"" + v->elem[0] + "\" src=\"" + v->elem[1] + "\" title=\"" + v->elem[2] + "\" />";
            flag = false;
        }

        //如果上面三者都不是, 则直接添加内容
        //FIXME:
        if (flag && v->type != 0)
        {
            content += v->elem[0];
            flag = false;
        }

        //递归遍历所有结点
        int vec_amount = (int)v->vec.size();
        int *ret = (int *)malloc(sizeof(int) * vec_amount);
        int record = 0;
        for (int i = 0; i < vec_amount; i++)
        {
            ret[i] = Cdfs(v->vec[i]);
            //TODO:输出该行前半部分的后置标签
            if (ret[i] == -1)
            {
                for (int j = i - 1; j >= 0; j--)
                    if (ret[j] > 0 && v->vec[j]->elem->size() == 0)
                        content += backTag[ret[j]];
                record = i + 1;
                content += v->vec[i]->elem[0];
            }
        }
        //拼接为结束标签
        //FIXME:
        if (!v->elem[0].empty())
        {
            content += backTag[v->type];
            //TODO:考虑“~~**混合1**~~、***混合2***”的情况
            if (v->type == 0) //如果该结点是nul型但elem[0]非空，说明结点后跟的是混合2的情况了，需要告知当前层要把前面的后置标签先输出
                return -1;
        }
        if (v->vec.size())
        {
            for (int i = vec_amount - 1; i >= record; i--)
                if (ret[i] > 0 && v->vec[i]->elem[0].size() == 0)
                    continue;
                    //content += backTag[ret[i]];
            content += backTag[v->type];
        }

        return v->type;
    }

    /*判断是否是分割线
     *分割线的markdown语有以下三种模式
     * "***"或"---"
     * "* * *"或"- - -"
     * "*****"或"----------"
     * FIXME:注意！！！如果"***"后面跟了文本，这一行就不是分割线了！！！
     */
    bool Deriver::isCutline(char *src)
    {
        int cnt = 0;
        char *ptr = src;
        //如果行首不是空格、tab、-或*, 则这一行不是分割线
        if (*ptr != ' ' && *ptr != '\t' && *ptr != '-' && *ptr != '*')
            return false;
        //模式一: 全部由'-'和空格/tab组成分割线
        while (*ptr)
        {
            if (*ptr != ' ' && *ptr != '\t' && *ptr != '-')
                break;
            if (*ptr == '-')
                cnt++;
            ptr++;
        }
        if (*ptr != '\n') //如果"___"后面跟了文本，这一行就不是分割线了
            return false;
        //模式二: 全部由'-'和空格/tab组成分割线
        if (cnt < 3)
        {
            cnt = 0;
            ptr = src;
            while (*ptr)
            {
                if (*ptr != ' ' && *ptr != '\t' && *ptr != '*')
                    break;
                if (*ptr == '*')
                    cnt++;
                ptr++;
            }
            if (*ptr != '\n') //如果"***"后面跟了文本，这一行就不是分割线了
                return false;
        }
        //如果出现三个以上的'-'或'*', 则需要增加一个分割线, 这时需要换行
        return (cnt >= 3);
    }

    void Deriver::makePara(Cnode *v)
    {
        if (v->vec.size() == 1u && v->vec.back()->type == paragraph)
            return;
        if (v->type == paragraph)
            return;
        if (v->type == nul)
        {
            v->type = paragraph;
            return;
        }
        Cnode *x = new Cnode(paragraph);
        x->vec = v->vec;
        v->vec.clear();
        v->vec.push_back(x);
    }

    /*行首的空格和tab不影响对markdown词法的解析
     *所以要先处理行首的空格和tab
     *src：要处理的一行字符串
     *返回: 由空格数和正文开始处的char*指针组成的pair类型
     */
    pair<int, char *> Deriver::tabcount(char *src)
    {
        //该行是空行
        if (strlen(src) == 0)
            return make_pair(0, nullptr);

        //从该行的第一个字符开始, 统计空格数和tab数, 遇到非空格且非tab时停止
        int spacecnt = 0, tabcnt = 0;
        for (int i = 0; src[i] != '\0'; i++)
        {
            if (src[i] == ' ')
                spacecnt++;
            else if (src[i] == '\t')
                tabcnt++;
            //如果内容前有空格和tab, 则统一按tab的个数处理, 一个tab = 4个空格
            else return make_pair(tabcnt + spacecnt / 4, src + i);
        }
        return make_pair(0, nullptr);
    }

    /*判断当前行的类型
     *src: 所要判断的行
     *返回: 当前行的类型和除去行标志性关键字的正式内容的char*组成的pair类型
     */
    pair<int, char *> Deriver::LineType(char *src)
    {
        char *ptr = src;

        //跳过`#`
        while (*ptr == '#')
            ptr++;
        //如果'#'紧跟空格, 则该行是标题
        if (ptr - src > 0 && *ptr == ' ')
            return make_pair(ptr - src + h1 - 1, ptr + 1); //返回标题级数

        //不是标题则从头分析
        ptr = src;

        //FIXME:需要判断是第一次还是第二次出现
        //如果出现```则说明是代码块
        if (strncmp(ptr, "```", 3) == 0)
            return make_pair(blockcode, ptr + 3);

        //FIXME:如果出现* + -, 并且它们的下一个字符为空格, 则该行是无序列表的一项
        if (strncmp(ptr, "- ", 2) == 0 || strncmp(ptr, "+ ", 2) == 0 || strncmp(ptr, "* ", 2) == 0)
            return make_pair(ul, ptr + 1);

        //如果出现 > 且下一个字符为空格, 则该行是引用
        if (strncmp(ptr, "> ", 2) == 0)
            return make_pair(quote, ptr + 1);

        //如果出现的是"数字. ", 则该行是有序列表的一项
        //FIXME:return是加1还是加2?
        char *ptr1 = ptr;
        while (*ptr1 && (isdigit(*ptr1)))
            ptr1++;
        if (ptr1 != ptr && strncmp(ptr1, ". ", 2) == 0)
            return make_pair(ol, ptr1 + 1);

        //否则就是普通段落
        return make_pair(paragraph, ptr);
    }

    //给定树的深度depth, 寻找结点, 返回结点指针
    Cnode * Deriver::findnode(int depth)
    {
        Cnode *ptr = Croot;
        while (!ptr->vec.empty() && depth != 0)
        {
            ptr = ptr->vec.back(); //返回尾元素的引用
            if (ptr->type == li)
                depth--;
        }
        return ptr;
    }

    /*新增结点的插入
     *结点的插入分两种类型
     *一种是向Tnode结构的树中插入目录结点
     *设置tag标签来标记一个目录结点所指向的正文结点, 
     *以实现点击目录项即可正确跳转到正文对应的位置
     */
    void Deriver::Tinsert(Tnode *v, int x, const string &headstr, int tag)
    {
        int n = v->vec.size(); //当前结点的vector项数
        if (x == 1)
        {
            v->vec.push_back(new Tnode(headstr)); //向量尾部增加一个结点
            v->vec.back()->tag = "tag" + to_string(tag);
            return;
        }
        if (!n || v->vec.back()->head.empty())
            v->vec.push_back(new Tnode(""));
        Tinsert(v->vec.back(), x - 1, headstr, tag);
    }

    void Deriver::Cinsert(Cnode *v, const string &src)
    {
        int srclen = src.size();
        bool incode = false,    //标志是否是行内代码
            inem = false,       //标志是否是行内斜体
            instrong = false,   //标志是否是行内加粗
            inautolink = false, //标志是否是行内链接
            indel = false,      //标志是否是删除线
            inunderline = false;//标志是否是下划线
        v->vec.push_back(new Cnode(nul));

        for (int i = 0; i < srclen; i++) //检查src字符串中的每一个字符
        {
            char ch = src[i];
            if (ch == '\\') //反斜杠的下一个字符即为转义字符
            {
                ch = src[++i];
                v->vec.back()->elem[0] += string(1, ch);
                continue;
            }

            //处理行内代码
            if (ch == '`' && !inautolink) //要求不是行内链接
            {
                if (incode)
                    v->vec.push_back(new Cnode(nul)); //如果incode为真，则当前ch是第二个'`'，增加一个空类型的结点
                else
                    v->vec.push_back(new Cnode(code)); //incode为假，当前ch是首个'`'，增加一个code类型的结点
                incode = !incode;
                continue;
            }

            //处理加粗，注意"***混合***”的情况
            if (ch == '*' && (i < srclen - 1 && (src[i + 1] == '*')) && !incode && !inautolink)
            { //读取到连续两个'*'，可能是加粗
                ++i;
                if (instrong)
                    v->vec.push_back(new Cnode(nul));
                else
                    v->vec.push_back(new Cnode(strong)); //如果instrong为假，则当前读到的连续两个'*'是首次，增加一个strong类型结点
                instrong = !instrong;
                continue;
            }
            if (ch == '_' && (i < srclen - 1 && (src[i + 1] == '_')) && !incode && !inautolink)
            { //读取到连续两个'_'，可能是加粗
                ++i;
                if (instrong)
                    v->vec.push_back(new Cnode(nul));
                else
                    v->vec.push_back(new Cnode(strong)); //如果instrong为假，则当前读到的连续两个'*'是首次，增加一个strong类型结点
                instrong = !instrong;
                continue;
            }

            //处理斜体
            //FIXME:
            //if (ch == '_' && !incode && !instrong && !inautolink)
            if (ch == '_' && src[i + 1] != '_' && !incode && !inautolink)
            { //只有单独的一个'_'，可能是斜体
                if (inem)
                    v->vec.push_back(new Cnode(nul));
                else
                    v->vec.push_back(new Cnode(em));
                inem = !inem;
                continue;
            }
            if (ch == '*' && src[i + 1] != '*' && !incode && !inautolink)
            { //只有单独的一个'*'，可能是斜体
                if (inem)
                    v->vec.push_back(new Cnode(nul));
                else
                    v->vec.push_back(new Cnode(em));
                inem = !inem;
                continue;
            }

            //处理删除线
            if (ch == '~' && (i < srclen - 1 && (src[i + 1] == '~')) && !incode && !inautolink)
            {
                ++i;
                //indel ? v->vec.push_back(new Cnode(nul)) : v->vec.push_back(new Cnode(del));
                if (indel)
                    v->vec.push_back(new Cnode(nul));
                else
                    v->vec.push_back(new Cnode(del));
                indel = !indel;
                continue;
            }

            //TODO:处理下划线
            if (ch == '<' && src[i + 1] == 'u' && src[i + 2] == '>' && !incode && !inautolink)
            {
                i = i + 2;
                if (inunderline)
                    v->vec.push_back(new Cnode(nul));
                else
                    v->vec.push_back(new Cnode(underline));
                inunderline = !inunderline;
                continue;
            }

            //处理图片
            if (ch == '!' && (i < srclen - 1 && src[i + 1] == '[') && !incode && !instrong && !inem && !inautolink)
            {
                v->vec.push_back(new Cnode(image));
                for (i += 2; i < srclen - 1 && src[i] != ']'; i++)
                    v->vec.back()->elem[0] += string(1, src[i]);
                i++;
                for (i++; i < srclen - 1 && src[i] != ' ' && src[i] != ')'; i++)
                    v->vec.back()->elem[1] += string(1, src[i]);
                if (src[i] != ')')
                    for (i++; i < srclen - 1 && src[i] != ')'; i++)
                        if (src[i] != '"')
                            v->vec.back()->elem[2] += string(1, src[i]);
                v->vec.push_back(new Cnode(nul));
                continue;
            }

            //处理超链接
            if (ch == '[' && !incode && !instrong && !inem && !inautolink)
            {
                v->vec.push_back(new Cnode(href));
                for (i++; i < srclen - 1 && src[i] != ']'; i++)
                    v->vec.back()->elem[0] += string(1, src[i]);
                i++;
                for (i++; i < srclen - 1 && src[i] != ' ' && src[i] != ')'; i++)
                    v->vec.back()->elem[1] += string(1, src[i]);
                if (src[i] != ')')
                    for (i++; i < srclen - 1 && src[i] != ')'; i++)
                        if (src[i] != '"')
                            v->vec.back()->elem[2] += string(1, src[i]);
                v->vec.push_back(new Cnode(nul));
                continue;
            }

            //ch读到的是普通文本
            v->vec.back()->elem[0] += string(1, ch);
            if (inautolink)
                v->vec.back()->elem[1] += string(1, ch);
        }
        if (src.size() >= 2)
            if (src.at(src.size() - 1) == ' ' && src.at(src.size() - 2) == ' ') //字符串最后连续两个空格
                v->vec.push_back(new Cnode(br));
    }

    Deriver::Deriver(const std::string &filename)
    {
        Troot = new Tnode("");
        Croot = new Cnode(nul);
        current = Croot;

        std::ifstream fin(filename);

        bool newpara = false;
        bool inblock = false;
        while (!fin.eof())
        {
            //从文件中获取一行
            fin.getline(line, MAX_LINE_LEN);
            //printf("start %s end\n", line);

            //处理不在代码块中且该行是分割线的情况
            if (!inblock && isCutline(line))
            {
                current = Croot;
                current->vec.push_back(new Cnode(hr));
                newpara = false;
                continue;
            }

            //计算一行中开始的空格和tab数
            std::pair<int, char *> ps = tabcount(line);

            //如果不是位于代码块中, 且ps.second为nulptr, 则直接读取下一行
            if (!inblock && ps.second == nullptr)
            {
                current = Croot;
                newpara = true;
                continue;
            }

            //分析该行文本的类型
            std::pair<int, char *> tp = LineType(ps.second);
            //printf("PSTART %s END\n", ps.second);

            //是代码块类型
            if (tp.first == blockcode)
            {
                //如果位于代码块中, 则push一个空类型的结点
                inblock ? current->vec.push_back(new Cnode(nul)) : current->vec.push_back(new Cnode(blockcode));
                inblock = !inblock;
                continue;
            }

            //如果位于代码块中, 直接将内容拼接到当前结点中
            if (inblock)
            {
                current->vec.back()->elem[0] += string(line) + '\n';
                continue;
            }

            //是普通段落
            //FIXME:删除线、粗体混用出错的地方
            if (tp.first == paragraph)
            {
                if (current == Croot)
                {
                    current = findnode(ps.first);
                    current->vec.push_back(new Cnode(paragraph));
                    current = current->vec.back();
                }
                bool flag = false;
                if (newpara && !current->vec.empty())
                {
                    Cnode *ptr = nullptr;
                    for (auto i : current->vec)
                    {
                        if (i->type == nul)
                            ptr = i;
                    }
                    if (ptr != nullptr)
                        makePara(ptr);
                    flag = true;
                }
                if (flag)
                {
                    current->vec.push_back(new Cnode(paragraph));
                    current = current->vec.back();
                }
                current->vec.push_back(new Cnode(nul));
                Cinsert(current->vec.back(), string(tp.second));
                //printf("duanluo%s \n", tp.second);
                newpara = false;
                continue;
            }

            current = findnode(ps.first);

            //如果是标题行, 则向其标签中插入属性tag
            if (tp.first >= h1 && tp.first <= h6)
            {
                current->vec.push_back(new Cnode(tp.first));
                current->vec.back()->elem[0] = "tag" + to_string(++cntTag);
                Cinsert(current->vec.back(), string(tp.second));
                Tinsert(Troot, tp.first - h1 + 1, string(tp.second), cntTag);
            }

            //如果是无序列表
            if (tp.first == ul)
            {
                if (current->vec.empty() || current->vec.back()->type != ul)
                {
                    current->vec.push_back(new Cnode(ul));
                }
                current = current->vec.back();
                current->vec.push_back(new Cnode(li));
                current = current->vec.back();
                Cinsert(current, string(tp.second));
            }
            //如果是有序列表
            if (tp.first == ol)
            {
                if (current->vec.empty() || current->vec.back()->type != ol)
                {
                    current->vec.push_back(new Cnode(ol));
                }
                current = current->vec.back();
                current->vec.push_back(new Cnode(li));
                current = current->vec.back();
                Cinsert(current, string(tp.second));
            }

            //如果是引用
            if (tp.first == quote)
            {
                if (current->vec.empty() || current->vec.back()->type != quote)
                {
                    current->vec.push_back(new Cnode(quote));
                }
                current = current->vec.back();
                if (newpara || current->vec.empty())
                    current->vec.push_back(new Cnode(paragraph));
                Cinsert(current->vec.back(), string(tp.second));
            }

            newpara = false;
        }

        //文件读取分析完毕
        fin.close();

        //深度优先遍历整棵语法树
        Cdfs(Croot);

        //构造目录
        TOC += "<ul>";
        for (int i = 0; i < (int)Troot->vec.size(); i++)
            Tdfs(Troot->vec[i], to_string(i + 1) + ".");
        TOC += "</ul>";
    }
#endif
