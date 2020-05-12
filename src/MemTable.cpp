//
// Created by Administrator on 2020/4/22.
//

#include "MemTable.h"
#include <future>
#include <algorithm>

//extern SSTManage sstM;// 全局唯一
SSTManage sstM=SSTManage("/tmp/tmpp/SSTManage");
int TableBase::insert(Data& data) {
    int dataSize=data.myByteSize();
    assert(getLeftSpace()>=dataSize);
    tbList_ptr->insert(data);
    ++dataCounter;
    listByteSize+=dataSize;
    return 0;
}

int TableBase::writeSSTable() {
    assert(imFlag==IMMUTABLE);
    //cout<<"SST Create!"<<endl;
    sstM.writeToSST(tbList_ptr->getIterator());
    auto p=tbList_ptr.release();
    delete p;
    tbList_ptr.reset(new SkipList());
    listByteSize=0;
    dataCounter=0;
    imFlag=MUTABLE;
}
Data TableBase::findGreatOrEqual(Data& data){
    auto node_ptr=(*tbList_ptr).findGreatOrEqual(data,nullptr);
    if(node_ptr){
        return (*node_ptr)._data;
    }else{
        return Data::getInvalidData();
    }
}

int MemTable::findTable() {
    for(int i=0;i<TABLE_NUMBER;i++){
        if(myTable[i].imFlag==TableBase::MUTABLE) return i;
    }
    return -1;
}

int MemTable::saveData(Data &data) {
    int pos=findTable();
    while(pos>=0){
        auto& tb=myTable[pos];
        if(tb.getLeftSpace()>=data.myByteSize()){
            tb.insert(data);
            return 0;
        }else{
            tb.setNextStatus();
            // save SST
            // todo 应该修改状态，开线程去写
            auto trd=async(std::launch::async,&TableBase::writeSSTable,&tb);// new thread to write SkipList data.
            pos=findTable();
        }
    }
    return pos;
}
Data MemTable::findGreatOrEqual(Data& data){
    vector<Data> vc;
    for(int i=0;i<TABLE_NUMBER;i++){
        Data dt1=myTable[i].findGreatOrEqual(data);
        if(!dt1.isInvalidData()){
            vc.push_back(dt1);
        }
    }
    sort(vc.begin(),vc.end(),[](Data dt1,Data dt2){return Data::compare(dt1,dt2);});
    if(vc.size()){
        return vc[0];
    }else{
        return Data::getInvalidData();
    }
}

void testMemTable(){
    // 先去main,定义全局变量sstM;  SSTManage sstM=SSTManage("/tmp/tmpp/SSTManage");
    MemTable mt;
    int N=100000;
    for(int i=0;i<N;i++){
        Data data(i,1,to_string(i),to_string(i));
        auto e=mt.saveData(data);
        if(e==-1){
            cout<<"### \t\t"<<i<<endl;
            break;
        }
    }
    /*测试memTable的查找方法
    Data dt(999999,1,"999999","");
    auto rt=mt.findGreatOrEqual(dt);
    cout<<"find"<<endl<<dt<<endl;
    cout<<"now"<<endl<<rt<<endl;*/
}