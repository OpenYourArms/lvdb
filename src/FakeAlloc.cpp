//
// Created by Administrator on 2020/4/14.
//
#include "FakeAlloc.h"

FakeAllocBase::_BlockNode* FakeAllocBase::_Block_List_Array[FakeAllocBase::_INDEX_SIZE];
char* FakeAllocBase::_poolStart=nullptr;
char* FakeAllocBase::_poolEnd=nullptr;
size_t FakeAllocBase::_usedSize=0;

FakeAlloc::pointer FakeAlloc::allocate(size_type sz){
    assert(sz>0);
    size_t align_size=roundUp(sz);
    pointer res=nullptr;
    if(align_size>_MAX_BLOCK){
        res=(pointer)::operator new(align_size);
    }else{
        int idx=getIndex(align_size);
        if(FakeAllocBase::_Block_List_Array[idx]){
            res=(pointer)FakeAllocBase::_Block_List_Array[idx];
            FakeAllocBase::_Block_List_Array[idx]=FakeAllocBase::_Block_List_Array[idx]->_nextBlock;
        }else{
            res=(pointer)reFill(align_size);
        }
        if(!res){
            throw bad_alloc();
        }
    }
    return res;
}

void FakeAlloc::deallocate(void* p,size_type sz){
    assert(p&&sz);
    size_t align_size=roundUp(sz);
    if(align_size>_MAX_BLOCK){
        ::operator delete(p);
    }else{
        int idx=getIndex(align_size);
        _BlockNode* node=(_BlockNode*)p;
        node->_nextBlock=FakeAllocBase::_Block_List_Array[idx];
        FakeAllocBase::_Block_List_Array[idx]=node;
    }
}

void test_FakeAlloc(){
    vector<int> vc{4*32,4*32-1,6*32,3*32,1023,1025,777,144};
    const int N=500;
    unordered_map<char*,size_t> mmp;
    FakeAlloc fa;
    for(int i=0;i<N;i++){
        int pos=rand()%vc.size();
        int sz=vc[pos];
        int fg=rand()%2;
        if(fg){
            auto e=mmp.begin();
            if(e!=mmp.end()){
                fa.deallocate((*e).first,(*e).second);
                mmp.erase(e);
            }
        }
        char* p=fa.allocate((size_t)sz);
        mmp[p]=(size_t)sz;
    }
    cout<<"MMP SIZE:"<<mmp.size()<<endl;
    for(auto e:mmp){
        fa.deallocate(e.first,e.second);
    }
}