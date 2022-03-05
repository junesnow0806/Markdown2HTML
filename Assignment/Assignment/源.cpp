#include <iostream>
#include <string>
#include <cmath>
#include <list>
#include <map>
#include <queue>
#include <vector>
#include <stack>
#include <fstream>
//#include"markdown_to_html_temp.h"
using namespace std;

enum Token
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
    image = 9,
    quote = 10,
    h1 = 11,
    h2 = 12,
    h3 = 13,
    h4 = 14,
    h5 = 15,
    h6 = 16,
    blockcode = 17,
    code = 18,
    un = 19,
    linethr = 20,
};

// HTML ǰ�ñ�ǩ
const std::string frontTag[] = {
    "", "<p>", "", "<ul>", "<ol>", "<li>", "<em>", "<strong>", "<hr color=#CCCCCC size=1 / > ",
    "", "<blockquote>", "<h1>", "<h2>", "<h3>", "<h4>", "<h5>", "<h6>",
    "<pre><code>", "<code>"};
// HTML ���ñ�ǩ
const std::string backTag[] = {
    "", "</p>", "", "</ul>", "</ol>", "</li>", "</em>",
    "</strong>", "", "", "</blockquote>", "</h1>", "</h2>",
    "</h3>", "</h4>", "</h5>", "</h6>", "</code></pre>", "</code>"};

struct Node
{
    //�﷨����
    int _type;
    //���ӽڵ�
    vector<Node *> _child;
    //����
    //elem[0]:������Ҫ��ʾ������
    //elem[1]:������ַ/·��
    string elem[2];

    Node(int type)
        : _type(type)
    {
    }
};

class MDtoHTML
{
public:
    MDtoHTML(const string &filename) : _root(NULL), _filename(filename){};
    const char *RemoveSpace(const char *str);             //ȥ�����׵Ŀո񣬷���֮���﷨����
    bool IsCutLine(const char *str);                      //�ж��Ƿ���ˮƽ�ָ���
    const char *DecodeType(const char *start, int *type); //�����﷨
    void Insert(Node *Node, char *str);                   //���ַ�����ڵ㣬strΪ��ǰ������
    void DFS(Node *root);                                 //������ȱ���root�������﷨��ת����HTML
    void Trans();                                         //�Դ����md�ļ����������﷨�����������﷨����չ�����������Ĳ��ֵ�htmlԴ����
    void Derivehtml();                                    //����html�ļ���head��end���֣���head�����ġ�end���ӡ�����html�ļ�
    void Destroy(Node *root);                             //�����﷨��
    ~MDtoHTML();

private:
    Node *_root;      //�﷨�����ڵ�
    string _filename; //�ļ���
    string _content;  //���HTML�ĵ�����
};

void MDtoHTML::Trans()
{
    ifstream fin(_filename);
    if (!fin)
    {
        cout << "Could not open this file!" << endl;
        exit(0);
    }
    else
    {
        bool block = false;
        string str;
        while (fin.peek() != EOF)
        {
            getline(fin, str);

            /*
             *  ȥ��ÿһ�е��׿ո�
            */

            const char *start = RemoveSpace(str.c_str());
            //������һ��ֻ�пո�,�������һ��,����һ�в����κδ���
            if (block == false && start == nullptr)
                continue;

            //�ж��Ƿ�Ϊˮƽ�ָ�
            if (block == false && IsCutLine(start) == true)
            {
                _root->_child.push_back(new Node(hr));
                continue;
            }

            /**
             *  ֮����жϽ�Ϊ�﷨���ж�
            **/

            //���ַ��������﷨����
            int type;
            const char *new_start = DecodeType(start, &type);
        }
    }
}

const char *MDtoHTML::DecodeType(const char *start, int *type)
{
    int temptype = 0;
    const char *s = start;
    if (*s == '#') //��һ������Ϊ������
    {
        temptype++;
        s++;
        for (; s != '\0' && *s == '#'; s++)
        {
            temptype++;
        }
        *type = h1 + temptype - 1;
        return s;
    }

    s = start;

    if (strncmp(s, "**", 2) == 0) //�ڶ�������Ϊ�Ӵ�
    {
        *type = strong;
        return s + 2;
    }

    if (strncmp(s, "*", 1) == 0) //����������Ϊб��
    {
        *type = em;
        return s + 1;
    }

    if (strncmp(s, "<u>", 3) == 0) //����������Ϊ�»���
    {
        *type = un;
        return s + 3;
    }

    if (strncmp(s, "~~", 2) == 0) //����������Ϊɾ����
    {
        *type = linethr;
        return s + 2;
    }

    if (strncmp(s, "`", 1) == 0) //����������Ϊ������
    {
        *type = code;
        return s + 1;
    }

    if (strncmp(s, "```", 3) == 0) //����������Ϊ�����
    {
        *type = blockcode;
        return s + 3;
    }

    if (strncmp(s, "- ", 2) == 0) //����������Ϊ�����б�
    {
        *type = ul;
        return s + 2;
    }

    if (*s >= '0' && *s <= '9') //�ڰ�������Ϊ�����б�
    {
        while (*s && (*s >= '0' && *s <= '9'))
            ++s;
        if (*s && *s == '.')
        {
            ++s;
            if (*s && *s == ' ')
            {
                *type = ul;
                return s + 2;
            }
        }
    }

    s = start;

    if (*s == '!' && *(s + 1) == '[')
    {
        //δ�꣬��ʱ����֪����ô�ֱ𱣴���ַ��˵��
    }
}

//�ݹ����ٺ���
void MDtoHTML::Destroy(Node *root)
{
    if (root)
    {
        for (Node *child : root->_child)
        {
            Destroy(child);
            root->_child.clear(); //�Ӳ����ƺ�����ν
        }

        delete root;
    }
}

MDtoHTML::~MDtoHTML()
{
    if (_root)
    {
        Destroy(_root);
    }
}

int main()
{
    MDtoHTML md("myfile.md"); //��Ҫת����markdown�ļ���Ϊ������ʵ����
    md.Trans();               //�������Ĳ��ֵ�htmlԴ����
    md.Derivehtml();          //����Ŀ��html�ļ�
    return 0;
}