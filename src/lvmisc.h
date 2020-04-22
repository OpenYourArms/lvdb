//
// Created by Administrator on 2020/4/17.
//

#ifndef LVDB_LVMISC_H
#define LVDB_LVMISC_H

#ifndef WIN32
#include <iostream>
#include <string>
#include <cstring>
#include <errno.h>
#include <iomanip>
#include <cassert>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
typedef unsigned long long ULL;

void showERROR(string str,bool ex=true);

struct Data{
    enum{DELETE=0,PUT=1,GET=2};
    string _key;
    string _value;
    ULL _sequenceNumber;
    int _op;
    Data()= default;
    Data(int seq,int op,string k,string v=""):_sequenceNumber(seq),_op(op),_key(k),_value(v){}
    friend ostream& operator<<(ostream& os,Data& a){
        return os<<setiosflags(ios::left)<<setw(4)<<a._key<<" : "<<a._value<<"\t"<<a._op<<"\t"<<a._sequenceNumber;
    }
    static bool compare(Data& a,Data& b){
        if(a._key!=b._key) return a._key<b._key;
        return a._sequenceNumber>b._sequenceNumber;
    }
};

template <typename T>
void toBuffer(char buffer[],int& pos,T obj){
    //cout<<"call T\t:"<<obj<<endl;
    int sz=sizeof(obj);
    memcpy(buffer+pos, &obj,sz);
    pos+=sz;
}

template <>//加结尾\0
void toBuffer(char buffer[],int& pos,const char* str);
template <>
void toBuffer(char buffer[],int& pos,char* str);
template <>//加结尾\0
void toBuffer(char buffer[],int& pos,string str);
void alignFileToMod(int fd,off_t mod);
void exFileSize(int fd,off_t size);

#endif

#endif //LVDB_LVMISC_H
