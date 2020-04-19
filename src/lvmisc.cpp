//
// Created by Administrator on 2020/4/17.
//
#include "lvmisc.h"


void showERROR(string str,bool ex){
    cout<<str<<endl;
    cout<<errno<<" : "<<strerror(errno)<<endl;
    if(ex) exit(3);
}

template <>
void toBuffer(char buffer[],int& pos,const char* cstr){
    int len=strlen(cstr);
    memcpy(buffer+pos,cstr,len);
    pos+=len;
    buffer[pos++]='\0';
}

template <>
void toBuffer(char buffer[],int& pos,string str){
    //cout<<"call str\t:"<<str<<endl;
    toBuffer(buffer,pos,str.c_str());
}
const int ALIGN_MOD=1024;
void alignFileToMod(int fd,off_t mod){
    struct stat fileStat;
    fstat(fd,&fileStat);
    off_t fsz=fileStat.st_size;
    char buf[ALIGN_MOD];
    memset(buf,0,sizeof(buf));
    int left=fsz%mod;
    if(left){
        pwrite(fd,buf,mod-left,fsz);
    }
}

void exFileSize(int fd,off_t size){
    struct stat fileStat;
    fstat(fd,&fileStat);
    off_t fsz=fileStat.st_size;
    int left=size-fsz;
    if(left<=0) return;
    char buf[ALIGN_MOD];
    memset(buf,0,sizeof(buf));
    int mini=0;
    while(left){
        mini=min(ALIGN_MOD,left);
        pwrite(fd,buf,mini,fsz);
        fsz+=mini;
        left-=mini;
    }
}