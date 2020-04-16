//
// Created by Administrator on 2020/4/14.
//

#ifndef LVDB_FAKEALLOC_H
#define LVDB_FAKEALLOC_H

#ifndef WIN32

#include <iostream>
#include <cassert>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstring>
using namespace std;

class FakeAllocBase{
public:
    enum {_MINI_BLOCK=32};
    enum {_BIT_SIZE=5};//2^_BIT_SIZE=_MINI_BLOCK;
    enum {_MAX_BLOCK=4096};
    enum {_INDEX_SIZE=(size_t)_MAX_BLOCK/(size_t)_MINI_BLOCK};
    enum {_INIT_SIZE=20};
    union _BlockNode{
        _BlockNode* _nextBlock;
    };

    static _BlockNode* _Block_List_Array[_INDEX_SIZE];
    static char* _poolStart;
    static char* _poolEnd;
    static size_t _usedSize;

    FakeAllocBase(){
        _poolStart=nullptr;
        _poolEnd=nullptr;
        _usedSize=0;
        memset(_Block_List_Array,0,sizeof(_Block_List_Array));
    }

    size_t roundUp(size_t bytes){
        /*if(bytes%_MINI_BLOCK){//模运算效率与除法相当，效率低
            bytes=bytes+_MINI_BLOCK;
        }*/
        return (bytes+(size_t)_MINI_BLOCK-1)&~((size_t)_MINI_BLOCK-1);
    }

    int getIndex(size_t bytes){
        assert(bytes>0);
        return (bytes-1)/(size_t)_MINI_BLOCK;
    }

    void insert(int idx,_BlockNode* node){
        node->_nextBlock=_Block_List_Array[idx];
        _Block_List_Array[idx]=node;
    }

    void* reFill(size_t bytes){
        char* res=nullptr;
        if(_poolStart+bytes<=_poolEnd){
            res=_poolStart;
            advance(_poolStart,1ll*bytes);
            return res;
        }else{
            size_t poolSize=bytes*_INIT_SIZE*2+(roundUp(_usedSize)>>_BIT_SIZE);
            _usedSize+=poolSize;
            if(_poolStart<_poolEnd){
                _BlockNode* leftBlock=(_BlockNode*)_poolStart;
                insert(getIndex(_poolEnd-_poolStart),leftBlock);
            }
            char *p=(char*)::operator new(poolSize);
            _poolStart=(char*)p;
            _poolEnd=_poolStart;
            advance(_poolStart,1ll*_INIT_SIZE*bytes);
            advance(_poolEnd,1ll*poolSize);
            res=p;
            char* tmp=nullptr;
            while(p<_poolStart){
                tmp=p;
                advance(p,1ll*bytes);
                ((_BlockNode*)tmp)->_nextBlock=(_BlockNode*)p;
            }
            int idx=getIndex(bytes);
            insert(idx,((_BlockNode*)res)->_nextBlock);
            return res;
        }
    }
};

class FakeAlloc:public FakeAllocBase{
public:
    typedef size_t size_type;
    typedef char* pointer;

    pointer allocate(size_type sz);
    void deallocate(void* p,size_type sz);
};

void test_FakeAlloc();


#endif

#endif //LVDB_FAKEALLOC_H
