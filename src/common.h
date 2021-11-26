/*
 * @Author: Corvo Attano(fkxzz001@qq.com)
 * @Description: 
 * @LastEditors: Corvo Attano(fkxzz001@qq.com)
 */
#pragma once
#include<string>
#include<vector>
#include<mutex>
#include<map>
#include<condition_variable>
#define MAX_BUFF_SIZE 4096
#define LAST_TIME_BUFF 256 //最后一次读取（超前读取）的buffer大小
extern int N;//子线程数
struct fileRecord
{
    std::string dentry;
    long int filesize;
};
struct workMem
{
    std::string filename;
};
//记录一个工作分割
struct splitRecord
{
    std::vector<struct fileRecord> fullFilelist;//工作分割中的完整文件表
    int cutFilecnt;//工作分割中的非完整文件数量
    struct fileRecord cutFile[2];//存放非完整文件的记录，一个工作分割中最多存在两个非完整文件
    long int cutFileoffset[2];//非完整文件的文件偏移起始地址
    long int cutFilelen[2];//非完整文件的长度
};
extern std::map<std::string,int> globalCounter;//全局计数器，收集各线程的统计结果
extern bool counterIdle;//全局计数器是否空闲
extern bool processed;//条件变量，是否已经完成
extern std::mutex mtx;//
extern std::condition_variable cv;
