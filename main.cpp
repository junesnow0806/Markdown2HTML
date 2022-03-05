#include <fstream>
#include "class.h"
//#include "MD.h"

int main()
{
    Deriver transformer("test_text2.md"); //装载目标Markdown文件

    std::string table = transformer.getTable(); //获取目标md文件的HTML格式的目录

    std::string contents = transformer.getContent(); //获取目标md文件转换成HTML格式后的内容

    //准备要写入的HTML文件的头尾信息
    //TODO:link的作用是什么
    std::string head = "<!DOCTYPE html><html><head>\
        <meta charset=\"utf-8\">\
        <title>Markdown</title>\
        <link rel=\"stylesheet\" href=\"github-markdown.css\">\
        </head><body><article class=\"markdown-body\">";
    std::string end = "</article></body></html>";

    //将结果写入到文件
    std::ofstream out;
    out.open("output/test_text2.html");
    out << head + table + contents + end; //将转换后的内容加到HTML头、尾内部
    out.close();
    return 0;
}
