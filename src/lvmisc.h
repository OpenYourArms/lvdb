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
    static Data getInvalidData(){ return Data(-1,-1,"","");}
    bool isInvalidData(){
        return _sequenceNumber==-1&&_op==-1&&_key==""&&_value=="";
    }
    int myByteSize(){// op 被压缩为char
        return sizeof(ULL) + sizeof(char) + strlen(_key.c_str())+1 + strlen(_value.c_str())+1;
    }
    void setToBuffer(char buf[],int& pos){
        // ULL,char,length1,key,length2,value;
        toBuffer(buf,pos,_sequenceNumber);
        char c=_op-0;
        toBuffer(buf,pos,c);
        //int len=strlen(_key.c_str())+1;
        toBuffer(buf,pos,_key);
        //len=strlen(_value.c_str())+1;
        toBuffer(buf,pos,_value);
    }
    void getFromBuffer(char buf[],int& pos){
        memcpy(&_sequenceNumber,buf+pos, sizeof(_sequenceNumber));
        pos+= sizeof(_sequenceNumber);
        char c;
        memcpy(&c,buf+pos, sizeof(char));
        pos+= sizeof(char);
        _op=c-0;
        _key=string(buf+pos);
        pos+=strlen(_key.c_str())+1;
        _value=string(buf+pos);
        pos+=strlen(_value.c_str())+1;
    }
};


#endif

#endif //LVDB_LVMISC_H
