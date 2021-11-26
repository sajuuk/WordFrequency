/*
 * @Author: Corvo Attano(fkxzz001@qq.com)
 * @Description: 
 * @LastEditors: Corvo Attano(fkxzz001@qq.com)
 */
#include<thread>
#include<unistd.h>
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<vector>
#include<dirent.h>
#include<sys/stat.h>
#include<chrono>
#include<ctime>
#include"fileReader.h"
#include"common.h"
#define MAX_PATH 256
using namespace std;
int N=8;
vector<fileRecord> fileList;
struct splitRecord *srList=nullptr;
long int totalSize=0;
std::map<std::string,int> globalCounter;
bool counterIdle=true;//全局计数器是否空闲
bool processed=false;//条件变量，是否已经完成
std::mutex mtx;//
std::condition_variable cv;
inline bool isTxt(char* name)
{
    char *dot;
    dot=name+(strlen(name) - 4);
    if(strcmp(dot,".txt")==0) return true;
    return false;
}
void dfs(const char *dir)
{
    DIR *dFd=opendir(dir);
    struct dirent *dentry;
    string name;
    struct stat stbuff;
    struct fileRecord fr;
    if(dFd==NULL)
    {
        fprintf(stderr,"can't open %s\n",dir);
        return;
    }
    while((dentry = readdir(dFd)) != NULL)
    {
        if(strcmp(dentry->d_name,".")==0 || strcmp(dentry->d_name,"..")==0)
        {
            continue;
        }
        name=string(dir)+'/'+string(dentry->d_name);
        if(stat(name.c_str(),&stbuff) == -1)
        {
            fprintf(stderr,"can't open %s\n",name.c_str());
            return;
        }
        if((stbuff.st_mode & S_IFMT) == S_IFDIR)
        {
            dfs(name.c_str());
        }
        else
        {
            if(!isTxt(dentry->d_name)) continue;
            fr.dentry=name;
            fr.filesize=stbuff.st_size;
            totalSize+=fr.filesize;
            fileList.push_back(fr);
        }
    }
    closedir(dFd);
}
void debugOutput(int index)
{
    printf("worker %d\n",index);
    for(auto iter=srList[index].fullFilelist.begin();iter != srList[index].fullFilelist.end();iter++)
    {
        printf("%s\n %ld\n",(*iter).dentry.c_str(),(*iter).filesize);
    }
    for(int i=0;i<srList[index].cutFilecnt;i++)
    {
        printf("%s \n %ld\n",srList[index].cutFile[i].dentry.c_str(),srList[index].cutFile[i].filesize);
        printf("begin: %ld len: %ld\n",srList[index].cutFileoffset[i],srList[index].cutFilelen[i]);
    }
}
void splitWork()
{
    int splitSize=totalSize/N + (totalSize%N>0);
    auto iter=fileList.begin();
    bool isCutfile=false;//iter指向的文件被分割
    int cutFileoffset;//被分割的文件当前的偏移量
    for(int i=0;i<N;i++)
    {
        srList[i].cutFilecnt=0;
        int rest=splitSize;//分配给该任务剩余的byte
        while(rest>0 && iter!=fileList.end())
        {
            if(isCutfile)//如果iter指向的文件被分割
            {
                int index=srList[i].cutFilecnt;
                srList[i].cutFilecnt++;
                srList[i].cutFileoffset[index] = cutFileoffset;
                srList[i].cutFile[index].dentry = (*iter).dentry;
                srList[i].cutFile[index].filesize = (*iter).filesize;
                if(rest >= (*iter).filesize - cutFileoffset)//如果剩余的容量能完全装下被分割的文件
                {
                    //设置所有srList项
                    srList[i].cutFilelen[index] = (*iter).filesize - cutFileoffset;
                    
                    isCutfile=false;
                    rest -= srList[i].cutFilelen[index];
                    cutFileoffset=0;
                    iter++;
                }
                else //如果剩余的容量不能装下被分割的文件
                {
                    //设置所有srList项
                    srList[i].cutFilelen[index] = rest;

                    cutFileoffset += rest;
                    rest = 0;
                }
            }
            else if(rest >= (*iter).filesize)//若能加入一个完整的文件，则加入
            {
                srList[i].fullFilelist.push_back(*iter);
                rest -= (*iter).filesize;
                iter++;
            }
            else
            {
                isCutfile = true;
                cutFileoffset = 0;
                continue;//标记当前iter指向的文件被分割，下一次循环中进行分割
            }
        }
        //debugOutput(i);
    }
}
void output(double timeUsed)
{
    printf("word frequency:\n");
    for(auto iter=globalCounter.begin();iter!=globalCounter.end();iter++)
    {
        printf("%s : %d\n",(*iter).first.c_str(),(*iter).second);
    }
    printf("time used: %lf \n",timeUsed);
}
int main(int argc,char **argv)
{
    chrono::steady_clock::time_point t1=chrono::steady_clock::now();
    char rootFile[MAX_PATH];
    rootFile[0]='.';
    if(argc < 2)
    {
        printf("usage %s -n [number of thread(s)] -d [dirname]\n",argv[0]);
        return 0;
    }
    for(int i=1;i<argc;i++)
    {
        if(!strcmp(argv[i],"-n"))
        {
            N=atoi(argv[++i]);
        }
        else if(!strcmp(argv[i],"-d"))
        {
            strcpy(rootFile,argv[++i]);
        }
        else
        {
            printf("usage %s -n [number of thread(s)] -d [dirname]\n",argv[0]);
            return 0;
        }
    }
    srList=new struct splitRecord[N];
    dfs(rootFile);
    splitWork();
    //fileReader *fR[N];
    //fileReader **fR=new *fileReader[N];
    fileReader *fR=new fileReader[N];
    for(int i=0;i<N;i++)
    {
        fR[i].init(&srList[i]);
    }
    thread *thd = new thread[N];
    for(int i=0;i<N;i++)
    {
        thd[i]=fR[i].workThread();
    }
    for(int i=0;i<N;i++)
    {
        thd[i].join();
    }
    chrono::steady_clock::time_point t2=chrono::steady_clock::now();
    chrono::duration<double> time_span=chrono::duration_cast<chrono::duration<double>>(t2-t1);
    double timeUsed=time_span.count();
    output(timeUsed);
    return 0;
}