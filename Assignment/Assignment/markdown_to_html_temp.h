#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

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

void MDtoHTML::Destroy()
{

}
