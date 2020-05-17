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
#include <map>
#include <algorithm>

using namespace std;
/*
struct Test{
    int _maxHeight;
    unsigned int _seed;// init
    char* _head;
    FakeAlloc fa;
    unordered_map<void*,int> _sizeMap;
    char* al(int sz){
        char* p=fa.allocate(sz);
        _sizeMap[p]=sz;
    }
    Test(){
        _head=(char*)al(15);
        _sizeMap[_head]=15;
    }
    ~Test(){
        cout<<_sizeMap.size()<<"\tsize"<<endl;
        for(auto e:_sizeMap){
            cout<<e.first<<"\t"<<e.second<<endl;
            fa.deallocate(e.first,e.second);
        }
    }
};*/
// SSTManage sstM=SSTManage("/tmp/tmpp/SSTManage");
extern SSTManage sstM;
int ok=0;
void prt(){
    cout<<"ok\t"<<ok<<endl;
}
int main() {
    std::cout << "Hi, World!" << std::endl;
//    testSSTable(1);
//    testSSTable(0);
    //testSSTableIterator();
    //test_SSTManage_with_SSTable();
//    test_SSTManage_read_write();
//    testMemTable();
//    testSSTableFind();
//    test_SSTManage_with_SSTable();

    atexit(prt);
    //初始化
    string journalPath="/tmp/tmpp/log.txt";
    Journal journal(journalPath);
    MemTable mt;
    // 存在在日志而不在文件的数据；
    if(!journal.empty()){
        ULL lastSeq=sstM.getMxSequenceNumber();
        //lastSeq为已存储的最大seq, _read寻找大于等于lastSeq.故+1
        vector<Data> unRecord=journal._read(lastSeq+1);
        cout<<"unRecord.size\t"<<unRecord.size()<<endl;
        for(auto& e:unRecord){
            if(e._op!=Data::GET){// get操作对memTable无用
                mt.saveData(e);
            }
        }
    }

    // 新数据
    string key="";
    int tms=10;
    int N=100000;

    int base=1000;
    clock_t t0,t1,sumt=0;
    cout<<"test begin"<<endl;
    sumt=0;
    for(int k=0;k<tms;k++){
        t0=clock();
        for(int i=0;i<N;i++){
//            int op1=Data::GET;
            int op1=rand()%3;
//            cout<<k<<"***"<<i<<endl;
//            key=to_string(base+i+(k*10));
            key=to_string(base+i);
            auto data=journal._write({op1,{key,key}});
            // 处理操作
            if(op1!=Data::GET){
                int rtNumber=mt.saveData(data);
            }else{
                Data rtData;
                rtData=mt.findGreatOrEqual(data);
                if(rtData._key!=data._key){
                    rtData=sstM.findGreatOrEqual(data);
                }
            }
        }
        sumt+=clock()-t0;
    }
    cout<<"put\t"<<N<<"\tcost time:\t"<<(sumt+1)/tms<<"\t=\t"<<((sumt+1)/tms)/(1000000)<<"s"<<endl;

//    cout<<"READ JOURNAL..."<<endl;
//    auto vcData=journal._read(0);
//    for(auto e:vcData){
//        cout<<e<<endl;
//    }
    return 0;
}

void testLvdb(vector<OPERATION_p>& vc){
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

    // 新数据
    for(int i=0;i<vc.size();i++){
        auto data=journal._write(vc[i]);
        cout<<data<<endl;
        // 处理操作
        int rtNumber=0;
        Data rtData;
        switch (data._op){
            case Data::DELETE:
            case Data::PUT:
                rtNumber=mt.saveData(data);
                if(rtNumber==0){// 成功
                    cout<<"OK, save Data:\t"<<data<<"\t"<<endl;
                }else{
                    cout<<"ERROR, Has no space for Data:\t"<<data<<"\t OK"<<endl;
                }
                break;
            case Data::GET: // 只有找到才有效，否则无效（返回无效数据/数据被delete）
                rtData=mt.findGreatOrEqual(data);
                if(rtData._key!=data._key){
                    rtData=sstM.findGreatOrEqual(data);
                }
                if(rtData._key==data._key&&rtData._op!=Data::DELETE){
                    cout<<"OK, search Data:\t"<<data<<",\tfind Data:\t"<<rtData<<endl;
                }else{
                    cout<<"ERROR, search Data:\t"<<data<<",\tfind Data:\t"<<rtData<<endl;
                }
                break;
            default:;
        }
    }
}
void testLeveldb(vector<OPERATION_p>& vc){

}