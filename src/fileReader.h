/*
 * @Author: Corvo Attano(fkxzz001@qq.com)
 * @Description: 
 * @LastEditors: Corvo Attano(fkxzz001@qq.com)
 */
#pragma once
#include <fstream>
#include <sstream>
#include <thread>
#include <map>
#include "common.h"
using namespace std;
class fileReader
{
private:
    char *buffer;
    char temp[256];
    ifstream fileInput;//文件流
    struct splitRecord *srList;//工作列表
    map<string,int> counter;//map 默认int型value为0
protected:
    void wordCounter(long int start,long int len);
    void debugOutput();
public:
    void work();
    thread workThread()
    {
        return thread(&fileReader::work,this);
    }
    fileReader();
    void init(struct splitRecord *ls);
    ~fileReader();
};  