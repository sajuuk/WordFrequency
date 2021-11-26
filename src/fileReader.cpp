/*
 * @Author: Corvo Attano(fkxzz001@qq.com)
 * @Description: 
 * @LastEditors: Corvo Attano(fkxzz001@qq.com)
 */
#include<cstdio>
#include<iostream>
#include<cstring>
#include"fileReader.h"
using namespace std;
fileReader::fileReader()
{

}
void fileReader::init(struct splitRecord *ls)
{
    this->srList=ls;
    buffer=(char*)malloc(MAX_BUFF_SIZE);
}
fileReader::~fileReader(void)
{
    free(buffer);
    fileInput.close();
}
static inline bool isChar(char a)
{
    if((a>='a' && a<='z') || (a>='A' && a<='Z')) return true;
    return false;
}
void fileReader::wordCounter(long int start,long int len)
{
    fileInput.seekg(start,fileInput.beg);
    fileInput.read(buffer,MAX_BUFF_SIZE);
    int pointer=0;
    int index=0;//temp的index
    int total=0;//当前已经统计的总byte数
    if(start)//如果不是从偏移量为0的位置开始读取
    {
        while (isChar(buffer[pointer])) 
        {
            pointer++;//舍弃第一个词
            total++;
        }
    }
    for(total;total<len;total++)
    {
        if(isChar(buffer[pointer]))
        {
            temp[index]=buffer[pointer];
            index++;
        }
        else
        {
            if(index)//temp中有数据，说明存在已经读入的词
            {
                temp[index]='\0';
                counter[string(temp)]++;
                index=0;
            }
        }
        pointer++;
        if(pointer == MAX_BUFF_SIZE)//如果读入到buffer末尾，则读入新数据
        {
            fileInput.read(buffer,MAX_BUFF_SIZE);
            pointer=0;
        }
    }
    //fileInput.seekg(start+len,fileInput.beg);
    fileInput.clear();//清除eof flag 以重定位
    fileInput.seekg(start+len);
    memset(buffer,0,LAST_TIME_BUFF);
    fileInput.read(buffer,LAST_TIME_BUFF);
    pointer=0;
    //若下一个块是以字母开始，则需要超前读取一个单词
    while(isChar(buffer[pointer]))
    {
        temp[index]=buffer[pointer];
        index++;
        pointer++;
    }
    if(index)
    {
        temp[index]='\0';
        counter[string(temp)]++;
        index=0;
    }
}
void fileReader::debugOutput()
{
    for(auto iter=counter.begin();iter!=counter.end();iter++)
    {
        printf("%s : %d\n",(*iter).first.c_str(),(*iter).second);
    }
}
void fileReader::work()
{
    if(fileInput.is_open()) fileInput.close();
    //处理完整文件
    for(auto iter=srList->fullFilelist.begin();iter !=srList->fullFilelist.end();iter++)
    {
        fileInput.open((*iter).dentry,ios::in);
        wordCounter(0,(*iter).filesize);
        fileInput.close();
        //debugOutput();
    }
    //处理分割文件
    for(int i=0;i<srList->cutFilecnt;i++)
    {
        fileInput.open(srList->cutFile[i].dentry,ios::in);
        wordCounter(srList->cutFileoffset[i],srList->cutFilelen[i]);
        fileInput.close();
        //debugOutput();
    }
    //临界区
    unique_lock<mutex> lck(mtx);
    cv.wait(lck,[](){return counterIdle;});
    counterIdle=false;
    for(auto iter=counter.begin();iter!=counter.end();iter++)
    {
        string key=(*iter).first;int value=(*iter).second;
        globalCounter[key]+=value;
    }
    counterIdle=true;
    lck.unlock();
    cv.notify_all();
}