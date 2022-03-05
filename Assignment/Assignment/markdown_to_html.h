#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

enum Token {
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
};

// HTML ǰ�ñ�ǩ
const std::string frontTag[] = {
 "", "<p>", "", "<ul>", "<ol>", "<li>", "<em>", "<strong>", "<hr color=#CCCCCC size=1 / > ",
 "", "<blockquote>", "<h1>", "<h2>", "<h3>", "<h4>", "<h5>", "<h6>",
 "<pre><code>", "<code>" };
// HTML ���ñ�ǩ
const std::string backTag[] = {
	"", "</p>", "", "</ul>", "</ol>", "</li>", "</em>",
	"</strong>", "", "", "</blockquote>", "</h1>", "</h2>",
	"</h3>", "</h4>", "</h5>", "</h6>", "</code></pre>", "</code>" };

struct Node {
	//�﷨����
	int _type;
	//���ӽڵ�
	vector<Node*> _child;
	//����
	//elem[0]:������Ҫ��ʾ������
	//elem[1]:������ַ/·��
	string elem[2];

	Node(int type)
		:_type(type)
	{}
};

class markdownPaerser {
public:
	markdownPaerser(const string& filename)
		:_root(new Node(nul))
		, _filename(filename)
	{}

	void transferm() {
		//���ļ�
		ifstream fin(_filename);
		if (!fin.is_open()) {	//��ʧ��
			cout << "���ļ�ʧ��" << endl;
			return;
		}

		bool inblock = false;
		//��ȡ����(���ж�ȡ)
		string rowStr;
		while (fin.peek() != EOF) {	//fin.eof()
			getline(fin, rowStr);

			//Ԥ����:�������׿ո�
			const char* start = processStr(rowStr.c_str());
			//���Կ�����
			if (!inblock && start == nullptr)	continue;

			//�ж��Ƿ�Ϊˮƽ�ָ���
			if (!inblock && isCutLine(start)) {
				_root->_child.push_back(new Node(hr));
				continue;
			}

			//�﷨����
			pair<int, const char*> typeRet = parseType(start);

			//�����﷨���

			//�������
			if (typeRet.first == blockcode) {
				//�жϴ������ʼ���ǽ���
				if (!inblock) {
					//�������ʼ
					//�����������
					_root->_child.push_back(new Node(blockcode));
				}

				//����Ǵ�������λ��,����Ҫ�����µĴ������

				inblock = !inblock;
				continue;
			}

			//�ж��Ƿ�Ϊ������д���
			if (inblock) {
				_root->_child.back()->elem[0] += rowStr;
				_root->_child.back()->elem[0] += '\n';
				continue;
			}

			//������
			if (typeRet.first == paragraph) {
				//����һ��������
				_root->_child.push_back(new Node(paragraph));
				//���ַ�����
				insert(_root->_child.back(), typeRet.second);
				continue;
			}

			//������
			if (typeRet.first >= h1 && typeRet.first <= h6) {
				//����������
				_root->_child.push_back(new Node(typeRet.first));
				//�����������
				_root->_child.back()->elem[0] = typeRet.second;
				continue;
			}

			//�����б�
			if (typeRet.first == ul) {
				//�ж��Ƿ�Ϊ�����б�ĵ�һ��
				//�ĵ���ʼ,�����﷨�����һ����㲻�������б���
				if (_root->_child.empty() || _root->_child.back()->_type != ul) {
					//���������б�
					_root->_child.push_back(new Node(ul));
				}
				//�������б�����б��ӽڵ�
				Node* uNode = _root->_child.back();
				uNode->_child.push_back(new Node(li));
				//���б��ӽڵ��������
				insert(uNode->_child.back(), typeRet.second);
				continue;
			}

			//�����б�
			if (typeRet.first == ol) {
				if (_root->_child.empty() || _root->_child.back()->_type != ol) {
					_root->_child.push_back(new Node(ol));
				}
				Node* oNode = _root->_child.back();
				oNode->_child.push_back(new Node(li));
				insert(oNode->_child.back(), typeRet.second);
				continue;
			}

			//����
			if (typeRet.first == quote) {
				//�������ý��
				_root->_child.push_back(new Node(quote));
				insert(_root->_child.back(), typeRet.second);
			}
		}
		//չ���﷨��,����HTML�ĵ�
		dfs(_root);
	}

	//�﷨��ת����HTMLԴ����
	//������ȱ��� DFS
	void dfs(Node* root) {
		//����ǰ�ñ�ǩ
		_content += frontTag[root->_type];

		//��������

		//��ַ
		if (root->_type == href) {
			_content += "<a href=\"";
			_content += root->elem[1];
			_content += "\">";
			_content += root->elem[0];
			_content += "</a>";
		}
		//ͼƬ
		else if (root->_type == image) {
			_content += "<img alt=\"";
			_content += root->elem[0];
			_content += "\" src=\"";
			_content += root->elem[1];
			_content += "\" />";
		}
		//����
		else {
			_content += root->elem[0];
		}

		//�����ӽ��
		for (Node* ch : root->_child) {
			dfs(ch);
		}

		//������ñ�ǩ
		_content += backTag[root->_type];
	}

	//ȥ�����׿ո�
	const char* processStr(const char* str) {
		while (*str) {
			if (*str == ' ' || *str == '\t')	++str;
			else break;
		}
		if (*str == '\0')	return nullptr;
		return str;
	}

	//���ַ����ݲ���
	void insert(Node* curNode, const char* str) {

		bool incode = false;	//�Ƿ�Ϊ���ڴ���
		bool instrong = false;	//����
		bool inem = false;		//б��
		int len = strlen(str);
		//��������Ϊ���ı�,���Է��봿�ı������
		//�ȴ���һ�����ı����ӽ��
		curNode->_child.push_back(new Node(nul));
		for (int i = 0; i < len; ++i) {
			//1.���ڴ���
			if (str[i] == '`') {
				if (incode) {
					//���ڴ������,�򴴽�һ���µĺ��ӽ��,��ź�������
					curNode->_child.push_back(new Node(nul));
				}
				else {
					//�������ڴ���
					curNode->_child.push_back(new Node(code));
				}
				incode = !incode;
				continue;
			}

			//2.����: "**  **"
			//����"**"�Ҳ������ڴ�����
			if (str[i] == '*' && i + 1 < len && str[i + 1] == '*' && !incode) {
				if (instrong) {
					//�������,�����½��
					curNode->_child.push_back(new Node(nul));
				}
				else {
					//�����µĴ�����
					curNode->_child.push_back(new Node(strong));
				}

				instrong = !instrong;
				//���������﷨
				++i;
				continue;
			}

			//3.б��:	"_   _"
			if (str[i] == '_' && !incode) {
				if (inem) {
					curNode->_child.push_back(new Node(nul));
				}
				else {
					curNode->_child.push_back(new Node(em));
				}
				inem = !inem;
				continue;
			}

			//4.ͼƬ:	![ͼƬ����](ͼƬ��ַ)
			if (str[i] == '!' && i + 1 < len && str[i + 1] == '[') {
				//����ͼƬ���
				curNode->_child.push_back(new Node(image));
				Node* iNode = curNode->_child.back();
				i += 2;
				//���ͼƬ������elem[0]
				for (; i < len && str[i] != ']'; ++i) {
					iNode->elem[0] += str[i];
				}
				//���ͼƬ��ַ��elem[1]
				i += 2;
				for (; i < len && str[i] != ')'; ++i) {
					iNode->elem[1] += str[i];
				}

				//�����½�㴦���������
				curNode->_child.push_back(new Node(nul));
				continue;
			}

			//5.����:[������](��ַ)
			//��������,�Ҳ������ڴ���
			if (str[i] == '[' && !incode) {
				//�������ӽ��
				curNode->_child.push_back(new Node(href));
				Node* hNode = curNode->_child.back();
				++i;
				//�����������elem[0]
				for (; i < len && str[i] != ']'; ++i) {
					hNode->elem[0] += str[i];
				}
				//������ӵ�ַ��elem[1]
				i += 2;
				for (; i < len && str[i] != ')'; ++i) {
					hNode->elem[1] += str[i];
				}

				//�����½�㴦���������
				curNode->_child.push_back(new Node(nul));
				continue;

			}

			//6.��ͨ���ı�
			curNode->_child.back()->elem[0] += str[i];
		}

	}
	//���������﷨  ������:�﷨���� + ��Ӧ������ʼλ��
	pair<int, const char*> parseType(const char* str) {
		//��������:# + �ո�
		const char* ptr = str;
		int titleNum = 0;
		while (*ptr && *ptr == '#') {
			++ptr;
			++titleNum;
		}
		//1.����
		if (*ptr == ' ' && titleNum > 0 && titleNum <= 6) {
			return make_pair(h1 + titleNum - 1, ptr + 1);
		}
		//�����ϱ����﷨,��Ҫ���½���
		ptr = str;

		//2.�����:```��������```
		if (strncmp(ptr, "```", 3) == 0) {
			return make_pair(blockcode, ptr + 3);
		}

		//3.�����б�: - + �ո�
		if (strncmp(ptr, "- ", 2) == 0) {
			return make_pair(ul, ptr + 2);
		}

		//4.�����б�:�����ַ� + "." + �ո�
		if (*ptr >= '0' && *ptr <= '9') {
			//����������
			while (*ptr && (*ptr >= '0' && *ptr <= '9'))	++ptr;
			if (*ptr && *ptr == '.') {
				++ptr;
				if (*ptr && *ptr == ' ')	return make_pair(ol, ptr + 1);
			}
		}
		//����
		ptr = str;
		//5.����:">" + �ո�
		if (strncmp(ptr, "> ", 2) == 0) {
			return make_pair(quote, ptr + 2);
		}

		//�����﷨ͳһ����Ϊ����
		return make_pair(paragraph, str);
	}
	//�ж�ˮƽ�ָ��� "---"
	bool isCutLine(const char* str) {
		int cnt = 0;
		while (*str && *str == '-') {
			++str;
			++cnt;
		}
		return cnt >= 3;
	}
	void html() {
		std::string head =
			"<!DOCTYPE html><html><head>\
        <meta charset=\"utf-8\">\
        <title>Markdown</title>\
        <link rel=\"stylesheet\" href=\"github-markdown.css\">\
        </head><body><article class=\"markdown-body\">";
		std::string end = "</article></body></html>";

		ofstream fout("out/markdown.html");
		fout << head << _content << end;
	}

	//����
	void destory(Node* root) {
		if (root) {
			for (Node* ch : root->_child) {
				destory(ch);
			}

			delete root;
		}
	}

	~markdownPaerser() {
		if (_root) {
			destory(_root);
		}
	}


private:
	//�﷨�����ڵ�
	Node* _root;
	//�ļ���
	string _filename;
	//���HTML�ĵ�����
	string _content;
};

