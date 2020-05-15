//
// Created by Administrator on 2020/5/15.
//
#include <iostream>
#include <ctime>
#include <cstring>
#include "src/Journal.h"
#include "src/SkipList.h"
#include "src/FakeAlloc.h"
#include "src/lvmisc.h"
#include "connect/link_cli.h"
#include "connect/link_server.h"
#include "src/SSTManage.h"
#include "src/SSTable.h"
#include "src/MemTable.h"

using namespace std;
const int MY_BUFFER_SIZE=256;
void lvdb_server();
void handleRequest(Journal& journal,MemTable& mt,int& fd);
string lvdb_query(Journal& journal,MemTable& mt,string& query);

extern SSTManage sstM;
int main(){
    lvdb_server();
    return 0;
}

void lvdb_server(){
    // Journal 写日志
    string journalPath="/tmp/tmpp/log.txt";
    Journal journal(journalPath);
    MemTable mt;
    // 存在在日志而不在文件的数据；
    if(!journal.empty()){
        ULL lastSeq=sstM.getMxSequenceNumber();
        //lastSeq为已存储的最大seq, _read寻找大于等于lastSeq.故+1
        vector<Data> unRecord=journal._read(lastSeq+1);
        for(auto& e:unRecord){
            if(e._op!=Data::GET){// get操作对memTable无用
                mt.saveData(e);
            }
        }
    }

    struct sockaddr_in cliaddr={};
    socklen_t clilen= sizeof(cliaddr);
    oyas::link_server server;
    cout<<"\t\t\t\t\tHandling..."<<endl;
    int fd=server.i_accept(&cliaddr,clilen);
    handleRequest(journal,mt,fd);
    // todo 开线程处理连接请求，长时间无操作结束线程
}
void handleRequest(Journal& journal,MemTable& mt,int& fd){
    string header="SERVER:\t";
    char buffer[MY_BUFFER_SIZE];
    strcpy(buffer,header.c_str());
    int pos= strlen(header.c_str());
    bool flag=true;
    while(flag){
        read(fd,buffer+pos, sizeof(buffer)-pos);
        string query(buffer+pos);
        if(query.empty()) continue;
        cout<<"get input\t:"<<query<<endl;
        string res="";
        if(query!="exit"){
            res=lvdb_query(journal,mt,query);
        }else{
            res="Bye.";
        }
        strcpy(buffer+pos,res.c_str());
        write(fd,buffer, sizeof(buffer));
        if(res=="Bye."){
            close(fd);
            fd=-2;
            break;
        }
    }
}
string lvdb_query(Journal& journal,MemTable& mt,string& query){
    string res="";
    string op="";
    auto result=parseString(query,op);
    if(result.first==-1){
        res="your input:\t"+query+"\thas some error. Please check it again.";
    }else{
        auto data=journal._write(result);
        cout<<data<<endl;
        // 处理操作
        int rtNumber=0;
        Data rtData;
        switch (data._op){
            case Data::DELETE:
                rtNumber=mt.saveData(data);
                if(rtNumber==0){// 成功
                    res="OK, delete Data:\t"+data._key+","+data._value;
                    cout<<"OK, save Data:\t"<<data<<"\t"<<endl;
                }else{
                    res="ERROR, Has no space for Data:\t"+data._key+","+data._value;
                    cout<<"ERROR, Has no space for Data:\t"<<data<<"\t OK"<<endl;
                }
                break;
            case Data::PUT:
                rtNumber=mt.saveData(data);
                if(rtNumber==0){// 成功
                    res="OK, save Data:\t"+data._key+","+data._value;
                    cout<<"OK, save Data:\t"<<data<<"\t"<<endl;
                }else{
                    res="ERROR, Has no space for Data:\t"+data._key+","+data._value;
                    cout<<"ERROR, Has no space for Data:\t"<<data<<"\t OK"<<endl;
                }
                break;
            case Data::GET: // 只有找到才有效，否则无效（返回无效数据/数据被delete）
                rtData=mt.findGreatOrEqual(data);
                if(rtData._key!=data._key){
                    rtData=sstM.findGreatOrEqual(data);
                }
                if(rtData._key==data._key&&rtData._op!=Data::DELETE){
                    res="OK, get Data:\t"+rtData._key+","+rtData._value;
                    cout<<"OK, search Data:\t"<<data<<",\tfind Data:\t"<<rtData<<endl;
                }else{
                    res="OK, not found this record\t"+data._key+","+data._value;
                    cout<<"ERROR, search Data:\t"<<data<<",\tfind Data:\t"<<rtData<<endl;
                }
                break;
            default:;
        }
    }
    return res;
}
