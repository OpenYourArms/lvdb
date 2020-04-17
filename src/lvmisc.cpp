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
    memcpy(buffer,cstr,len);
    pos+=len;
    buffer[pos++]='\0';
}

template <>
void toBuffer(char buffer[],int& pos,string str){
    cout<<"call str\t:"<<str<<endl;
    toBuffer(buffer,pos,str.c_str());
}

