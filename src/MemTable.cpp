//
// Created by Administrator on 2020/4/22.
//

#include "MemTable.h"

int TableBase::insert(Data& data) {
    int dataSize=data.myByteSize();
    assert(getLeftSpace()>=dataSize);
    tbList.insert(data);
    ++dataCounter;
    listByteSize+=dataSize;
    return 0;
}

int TableBase::writeSSTable() {
    assert(imFlag==IMMUTABLE);
    //cout<<"SST Create!"<<endl;
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
            tb.writeSSTable();// new thread to write SkipList data.
            pos=findTable();
        }
    }
    return pos;
}