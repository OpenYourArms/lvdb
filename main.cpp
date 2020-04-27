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
#include <map>
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
int main() {
    std::cout << "Hello, World!" << std::endl;
//    testSSTable(1);
//    testSSTable(0);
    test_SSTManage_with_SSTable();
    //test_SSTManage_read_write();
    return 0;
}
