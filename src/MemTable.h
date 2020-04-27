//
// Created by Administrator on 2020/4/22.
//

#ifndef LVDB_MEMTABLE_H
#define LVDB_MEMTABLE_H

#ifndef WIN32

#include "lvmisc.h"
#include "SkipList.h"
#include "SSTManage.h"
#include <memory>

struct TableBase{//一个 memTable
    enum {MUTABLE=true,IMMUTABLE=false};
    // todo size最大值待设置
    enum {TABLE_MAX_SIZE=100};
    unique_ptr<SkipList> tbList_ptr;
    int listByteSize;
    bool imFlag;
    int dataCounter;
    TableBase():tbList_ptr(new SkipList()),listByteSize(0),imFlag(MUTABLE),dataCounter(0){}
    inline int getLeftSpace(){return TABLE_MAX_SIZE-listByteSize;}
    inline void setNextStatus(){imFlag=!imFlag;}
    int insert(Data& data);// 返回码对应字符车串
    int writeSSTable();

};
class MemTable{
    enum {TABLE_NUMBER=2};
public:
    TableBase myTable[TABLE_NUMBER];
public:
    int findTable();//-1 代表没表可用
    int saveData(Data& data);
};

#endif

#endif //LVDB_MEMTABLE_H
