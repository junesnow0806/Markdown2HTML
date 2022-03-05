#ifndef MD2HTML //��ֹͷ�ļ����ظ������ͱ���
#define MD2HTML

#include <cstdlib>
#include <fstream>
#include <vector>
#include <cstring>
#include <utility>
#include <string>
#include <cctype>
#include <cstdio>
using namespace std;

#define maxLength 10000

// �ʷ��ؼ���ö��
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
    code = 19,
    del = 20
};
// HTML ǰ�ñ�ǩ
const string Head[] = {
    "", "<p>", "", "<ul>", "<ol>", "<li>", "<em>", "<strong>",
    "<hr color=#CCCCCC size=1 />", "<br />",
    "", "<blockquote>",
    "<h1 ", "<h2 ", "<h3 ", "<h4 ", "<h5 ", "<h6 ", // �ұߵļ�����Ԥ�������������ı�ǩ����
    "<pre><code>", "<code>", "<del>"};
// HTML ���ñ�ǩ
const string End[] = {
    "", "</p>", "", "</ul>", "</ol>", "</li>", "</em>", "</strong>",
    "", "", "", "</blockquote>",
    "</h1>", "</h2>", "</h3>", "</h4>", "</h5>", "</h6>",
    "</code></pre>", "</code>", "</del>"};
typedef struct Tnode
{
    vector<Tnode *> ch;
    string heading;
    string tag;
    Tnode(const string &hd) : heading(hd) {}
} Tnode;

typedef struct Cnode
{
    int type; // �ڵ����������
    vector<Cnode *> ch;
    string elem[2]; // �������������Ҫ������, elem[0] ������Ҫ��ʾ������, elem[1] ����������
    Cnode(int _type) : type(_type) {}
} Cnode;

class MarkdownTransform
{
private:
    Cnode *Croot, *now;
    Tnode *Troot;
    string content, TOC; //���ݺ�Ŀ¼
    int cntTag = 0;
    char s[maxLength];

    // �ж��Ƿ�Ϊ����
    inline bool isHeading(Cnode *v)
    {
        return (v->type >= h1 && v->type <= h6);
    }
    // �ж��Ƿ�ΪͼƬ
    inline bool isImage(Cnode *v)
    {
        return (v->type == image);
    }
    // �ж��Ƿ�Ϊ������
    inline bool isHref(Cnode *v)
    {
        return (v->type == href);
    }
    // �ݹ��������ڵ�
    template <typename T> //����ģ��
    void Destroy(T *v);

    //Ŀ¼������ȱ���
    void Tdfs(Tnode *v, string index);

    //��Tnode�ṹ�в���Ŀ¼�ڵ�
    void Tinsert(Tnode *v, int x, const string &hd, int tag);

    // ��ָ���Ľڵ��в���Ҫ�����Ĵ������Ĳ��֣�
    // v: �ڵ�
    // src: Ҫ�����Ĵ�
    void Cinsert(Cnode *v, const string &src);

    //����������ȱ���
    void Cdfs(Cnode *v);

    // �ж��Ƿ���
    inline bool IsCutLine(char *src);

    // ���ɶ���
    inline void mkpara(Cnode *v);

    // ��ʼ����һ���п�ʼ�Ŀո�� Tab
    // src: Դ��
    // ����ֵ: �ɿո����������ݴ��� char* ָ����ɵ� std::pair
    inline pair<int, char *> Removespace(char *src);

    // �жϵ�ǰ�е�����
    // src: Դ��
    // ����ֵ: ��ǰ�е����ͺͳ�ȥ�б�־�Թؼ��ֵ��������ݵ� char* ָ����ɵ� std::pair
    inline pair<int, char *> JudgeType(char *src);

    // �����������Ѱ�ҽڵ�
    // depth: �������
    // ����ֵ: �ҵ��Ľڵ�ָ��
    inline Cnode *findnode(int depth);

public:
    // ���캯��
    MarkdownTransform(const std::string &filename);

    // ��� Markdown Ŀ¼
    string getTableOfContents() { return TOC; }
    // ��� Markdown ����
    string getContents() { return content; }

    // ��������
    ~MarkdownTransform();
};

template <typename T>
void MarkdownTransform::Destroy(T *v)
{
    for (int i = 0; i < (int)v->ch.size(); i++)
    {
        Destroy(v->ch[i]);
    }
    delete v;
}

void MarkdownTransform::Tdfs(Tnode *v, string index)
{
    TOC += "<li>\n";
    TOC += "<a href=\"#" + v->tag + "\">" + index + " " + v->heading + "</a>\n";
    int n = (int)v->ch.size();
    if (n)
    {
        TOC += "<ul>\n";
        for (int i = 0; i < n; i++)
        {
            Tdfs(v->ch[i], index + to_string(i + 1) + ".");
        }
        TOC += "</ul>\n";
    }
    TOC += "</li>\n";
}

void MarkdownTransform::Tinsert(Tnode *v, int x, const string &hd, int tag)
{
    int n = (int)v->ch.size();
    if (x == 1)
    {
        v->ch.push_back(new Tnode(hd));
        v->ch.back()->tag = "tag" + to_string(tag);
        return;
    }

    if (!n || v->ch.back()->heading.empty())
        v->ch.push_back(new Tnode(""));
    Tinsert(v->ch.back(), x - 1, hd, tag);
}

void MarkdownTransform::Cdfs(Cnode *v)
{
    if (v->type == paragraph && v->elem[0].empty() && v->ch.empty())
        return;

    content += Head[v->type];
    bool flag = true;

    // ��������, ֧����Ŀ¼������ת
    if (isHeading(v))
    {
        content += "id=\"" + v->elem[0] + "\">";
        flag = false;
    }

    // ����������
    if (isHref(v))
    {
        content += "<a href=\"" + v->elem[1] + "\">" + v->elem[0] + "</a>";
        flag = false;
    }

    // ����ͼƬ
    if (isImage(v))
    {
        content += "<img alt=\"" + v->elem[0] + "\" src=\"" + v->elem[1] "\" />";
        flag = false;
    }

    // ����������߶�����, ��ֱ����������
    if (flag)
    {
        content += v->elem[0];
        flag = false;
    }

    // �ݹ��������
    for (int i = 0; i < (int)v->ch.size(); i++)
        Cdfs(v->ch[i]);

    // ƴ��Ϊ������ǩ
    content += End[v->type];
}

bool MarkdownTransform::IsCutLine(char *src)
{
    int cnt = 0;
    char *ptr = src;
    while (*ptr)
    {
        // ������� �ո�tab��- �� *����ô����Ҫ����
        if (*ptr != ' ' && *ptr != '\t' && *ptr != '-')
            return false;
        if (*ptr == '-')
            cnt++;
        ptr++;
    }
    // ������� --- ����Ҫ����һ���ָ���, ��ʱ��Ҫ����
    return (cnt >= 3);
}

void MarkdownTransform::mkpara(Cnode *v)
{
    if (v->ch.size() == 1u && v->ch.back()->type == paragraph)
        return;
    if (v->type == paragraph)
        return;
    if (v->type == nul)
    {
        v->type = paragraph;
        return;
    }
    Cnode *x = new Cnode(paragraph);
    x->ch = v->ch;
    v->ch.clear();
    v->ch.push_back(x);
}

pair<int, char *> MarkdownTransform::Removespace(char *src)
{
    // �����������Ϊ�գ���ֱ�ӷ���
    if ((int)strlen(src) == 0)
        return make_pair(0, nullptr);
    // ͳ�ƿո���� Tab ���ĸ���
    int cntspace = 0, cnttab = 0;
    // �Ӹ��еĵ�һ���ַ�����, ͳ�ƿո���� Tab ��,
    // ���������ǿո�� Tab ʱ������ֹͣ
    for (int i = 0; src[i] != '\0'; i++)
    {
        if (src[i] == ' ')
            cntspace++;
        else if (src[i] == '\t')
            cnttab++;
        // �������ǰ�пո�� Tab����ô����ͳһ�� Tab �ĸ�������,
        // ����, һ�� Tab = �ĸ��ո�
        return make_pair(cnttab + cntspace / 4, src + i);
    }
    return make_pair(0, nullptr);
}

pair<int, char *> MarkdownTransform::JudgeType(char *src)
{
    char *ptr = src;

    // ���� `#`
    while (*ptr == '#')
        ptr++;

    // ������ֿո�, ��˵���� `<h>` ��ǩ
    if (ptr > src && *ptr == ' ')
        return make_pair(ptr - src + h1 - 1, ptr + 1);

    // ���÷���λ��
    ptr = src;

    // ������� ``` ��˵���Ǵ����
    if (strncmp(ptr, "```", 3) == 0)
        return make_pair(blockcode, ptr + 3);

    // ������� * + -, �������ǵ���һ���ַ�Ϊ�ո���˵�����б�
    if (strncmp(ptr, "- ", 2) == 0 || strncmp(ptr, "+ ", 2) == 0 || strncmp(ptr, "* ", 2) == 0)
        return make_pair(ul, ptr + 1);

    // ������� > ����һ���ַ�Ϊ�ո���˵��������
    if (*ptr == '>' && (ptr[1] == ' '))
        return make_pair(quote, ptr + 1);

    // ������ֵ�������, ����һ���ַ��� . ��˵�����������б�
    char *ptr1 = ptr;
    while (*ptr1 && (isdigit(*ptr1)))
        ptr1++;
    if (ptr1 > ptr && *ptr1 == '.' && ptr1[1] == ' ')
        return make_pair(ol, ptr1 + 1);

    // ���򣬾�����ͨ����
    return make_pair(paragraph, ptr);
}

Cnode *MarkdownTransform::findnode(int depth)
{
    Cnode *ptr = Croot;
    while (!ptr->ch.empty() && depth != 0)
    {
        ptr = ptr->ch.back();
        if (ptr->type == li)
            depth--;
    }
    return ptr;
}

void MarkdownTransform::Cinsert(Cnode *v, const string &src)
{
    int n = (int)src.size();
    bool incode = false,
         inem = false,
         instrong = false,
         indel = false,
         inautolink = false;
    v->ch.push_back(new Cnode(nul));

    for (int i = 0; i < n; i++)
    {
        char ch = src[i];
        if (ch == '\\')
        {
            ch = src[++i];
            v->ch.back()->elem[0] += string(1, ch);
            continue;
        }

        // �������ڴ���
        if (ch == '`' && !inautolink)
        {
            incode ? v->ch.push_back(new Cnode(nul)) : v->ch.push_back(new Cnode(code));
            incode = !incode;
            continue;
        }

        // �����Ӵ�
        if (ch == '*' && (i < n - 1 && (src[i + 1] == '*')) && !incode && !inautolink)
        {
            ++i;
            instrong ? v->ch.push_back(new Cnode(nul)) : v->ch.push_back(new Cnode(strong));
            instrong = !instrong;
            continue;
        }
        //����б��
        if (ch == '_' && !incode && !instrong && !inautolink)
        {
            inem ? v->ch.push_back(new Cnode(nul)) : v->ch.push_back(new Cnode(em));
            inem = !inem;
            continue;
        }
        //����ɾ����
        if (ch == '~' && (i < n - 1 && (src[i + 1] == '~')) && !incode && !inautolink)
        {
            ++i;
            indel ? v->ch.push_back(new Cnode(nul)) : v->ch.push_back(new Cnode(del));
            indel = !indel;
            continue;
        }

        // ����ͼƬ
        if (ch == '!' && (i < n - 1 && src[i + 1] == '[') && !incode && !instrong && !inem && !inautolink)
        {
            v->ch.push_back(new Cnode(image));
            for (i += 2; i < n - 1 && src[i] != ']'; i++)
                v->ch.back()->elem[0] += string(1, src[i]);
            i++;
            for (i++; i < n - 1 && src[i] != ' ' && src[i] != ')'; i++)
                v->ch.back()->elem[1] += string(1, src[i]);
            //if (src[i] != ')')
            //    for (i++; i < n - 1 && src[i] != ')'; i++)
            //        if (src[i] != '"')
            //            v->ch.back()->elem[2] += string(1, src[i]);
            v->ch.push_back(new Cnode(nul));
            continue;
        }

        // ����������
        if (ch == '[' && !incode && !instrong && !inem && !inautolink)
        {
            v->ch.push_back(new Cnode(href));
            for (i++; i < n - 1 && src[i] != ']'; i++)
                v->ch.back()->elem[0] += string(1, src[i]);
            i++;
            for (i++; i < n - 1 && src[i] != ' ' && src[i] != ')'; i++)
                v->ch.back()->elem[1] += string(1, src[i]);
            //if (src[i] != ')')
            //    for (i++; i < n - 1 && src[i] != ')'; i++)
            //        if (src[i] != '"')
            //            v->ch.back()->elem[2] += string(1, src[i]);
            v->ch.push_back(new Cnode(nul));
            continue;
        }

        v->ch.back()->elem[0] += string(1, ch);
        if (inautolink)
            v->ch.back()->elem[1] += string(1, ch);
    }
    if (src.size() >= 2)
        if (src.at(src.size() - 1) == ' ' && src.at(src.size() - 2) == ' ')
            v->ch.push_back(new Cnode(br));
}

MarkdownTransform::MarkdownTransform(const std::string &filename)
{
    Troot = new Tnode("");
    Croot = new Cnode(nul);
    now = Croot;

    std::ifstream fin(filename);

    bool newpara = false;
    bool inblock = false;
    while (!fin.eof())
    {
        // ���ļ��л�ȡһ��
        fin.getline(s, maxLength);

        // �������ڴ��������Ҫ���е����
        if (!inblock && IsCutLine(s))
        {
            now = Croot;
            now->ch.push_back(new Cnode(hr));
            newpara = false;
            continue;
        }

        // std::pair ʵ������һ���ṹ��, ���Խ�����������ϳ�һ������
        // ����һ���п�ʼ�Ŀո�� Tab ��
        std::pair<int, char *> ps = Removespace(s);

        // ���û��λ�ڴ������, ��û��ͳ�Ƶ��ո�� Tab, ��ֱ�Ӷ�ȡ��һ��
        if (!inblock && ps.second == nullptr)
        {
            now = Croot;
            newpara = true;
            continue;
        }

        // ���������ı�������
        std::pair<int, char *> tj = JudgeType(ps.second);

        // ����Ǵ��������
        if (tj.first == blockcode)
        {
            // ���λ�ڴ������, �� push һ�������͵Ľڵ�
            inblock ? now->ch.push_back(new Cnode(nul)) : now->ch.push_back(new Cnode(blockcode));
            inblock = !inblock;
            continue;
        }

        // ����ڴ������, ֱ�ӽ�����ƴ�ӵ���ǰ�ڵ���
        if (inblock)
        {
            now->ch.back()->elem[0] += string(s) + '\n';
            continue;
        }

        // �������ͨ����
        if (tj.first == paragraph)
        {
            if (now == Croot)
            {
                now = findnode(ps.first);
                now->ch.push_back(new Cnode(paragraph));
                now = now->ch.back();
            }
            bool flag = false;
            if (newpara && !now->ch.empty())
            {
                Cnode *ptr = nullptr;
                for (auto i : now->ch)
                {
                    if (i->type == nul)
                        ptr = i;
                }
                if (ptr != nullptr)
                    mkpara(ptr);
                flag = true;
            }
            if (flag)
            {
                now->ch.push_back(new Cnode(paragraph));
                now = now->ch.back();
            }
            now->ch.push_back(new Cnode(nul));
            Cinsert(now->ch.back(), string(tj.second));
            newpara = false;
            continue;
        }

        now = findnode(ps.first);

        // ����Ǳ�����, �������ǩ�в������� tag
        if (tj.first >= h1 && tj.first <= h6)
        {
            now->ch.push_back(new Cnode(tj.first));
            now->ch.back()->elem[0] = "tag" + to_string(++cntTag);
            Cinsert(now->ch.back(), string(tj.second));
            Tinsert(Troot, tj.first - h1 + 1, string(tj.second), cntTag);
        }

        // ����������б�
        if (tj.first == ul)
        {
            if (now->ch.empty() || now->ch.back()->type != ul)
            {
                now->ch.push_back(new Cnode(ul));
            }
            now = now->ch.back();
            bool flag = false;
            if (newpara && !now->ch.empty())
            {
                Cnode *ptr = nullptr;
                for (auto i : now->ch)
                {
                    if (i->type == li)
                        ptr = i;
                }
                if (ptr != nullptr)
                    mkpara(ptr);
                flag = true;
            }
            now->ch.push_back(new Cnode(li));
            now = now->ch.back();
            if (flag)
            {
                now->ch.push_back(new Cnode(paragraph));
                now = now->ch.back();
            }
            Cinsert(now, string(tj.second));
        }

        // ����������б�
        if (tj.first == ol)
        {
            if (now->ch.empty() || now->ch.back()->type != ol)
            {
                now->ch.push_back(new Cnode(ol));
            }
            now = now->ch.back();
            bool flag = false;
            if (newpara && !now->ch.empty())
            {
                Cnode *ptr = nullptr;
                for (auto i : now->ch)
                {
                    if (i->type == li)
                        ptr = i;
                }
                if (ptr != nullptr)
                    mkpara(ptr);
                flag = true;
            }
            now->ch.push_back(new Cnode(li));
            now = now->ch.back();
            if (flag)
            {
                now->ch.push_back(new Cnode(paragraph));
                now = now->ch.back();
            }
            Cinsert(now, string(tj.second));
        }

        // ���������
        if (tj.first == quote)
        {
            if (now->ch.empty() || now->ch.back()->type != quote)
            {
                now->ch.push_back(new Cnode(quote));
            }
            now = now->ch.back();
            if (newpara || now->ch.empty())
                now->ch.push_back(new Cnode(paragraph));
            Cinsert(now->ch.back(), string(tj.second));
        }

        newpara = false;
    }

    // �ļ���ȡ�������
    fin.close();

    // �������ȱ��������﷨��
    Cdfs(Croot);

    // ����Ŀ¼
    TOC += "<ul>";
    for (int i = 0; i < (int)Troot->ch.size(); i++)
        Tdfs(Troot->ch[i], to_string(i + 1) + ".");
    TOC += "</ul>";
}

MarkdownTransform::~MarkdownTransform()
{
    Destroy<Cnode>(Croot);
    Destroy<Tnode>(Troot);
}
#endif
