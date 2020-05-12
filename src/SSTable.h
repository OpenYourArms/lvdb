//
// Created by Administrator on 2020/4/23.
//

#ifndef LVDB_SSTABLE_H
#define LVDB_SSTABLE_H

#ifndef WIN32

#include "lvmisc.h"
#include "SkipList.h"
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

// 实现没有布隆过滤器版本
class SSTable{
    enum {BLOCK_MAX_SIZE=4096};
public:
    struct BlockStruct{
        enum {SLICE_MAX_SIZE=BLOCK_MAX_SIZE/16};
        struct SliceIndex{
            int sliceOffset;
            int used;
            Data maxData;
            explicit SliceIndex(int offset,int sz):sliceOffset(offset),used(sz){}
            void addSize(int sz){used+=sz;}
            void setData(Data& data){maxData=data;}
            void setToBuffer(char buf[],int& pos){
                toBuffer(buf,pos,sliceOffset);
                toBuffer(buf,pos,used);
                maxData.setToBuffer(buf,pos);
            }
            void getFromBuffer(char buf[],int& pos){
                memcpy(&sliceOffset,buf+pos, sizeof(sliceOffset));
                pos+= sizeof(sliceOffset);
                memcpy(&used,buf+pos, sizeof(used));
                pos+= sizeof(used);
                maxData.getFromBuffer(buf,pos);
            }
        };
        vector<Data> dataVector;
        vector<SliceIndex> sliceVector;
        int usedSize;
        int maxDataSize;
        BlockStruct():usedSize(0),maxDataSize(0){}
        bool addData(Data& data){
            int sz=data.myByteSize();
            maxDataSize=max(sz,maxDataSize);
            int paddsz=sz + (sizeof(int)+sizeof(int)+maxDataSize)*(sliceVector.size()+1) + sizeof(int);// data + slice + indexPoint
            if(usedSize+paddsz>BLOCK_MAX_SIZE){
                auto& slice=sliceVector.back();
                slice.maxData=dataVector.back();
                return false;
            }
            if(dataVector.empty()){
                dataVector.push_back(data);
                sliceVector.push_back(SliceIndex(usedSize,sz));
                usedSize+=sz;
            }else{
                auto& slice=sliceVector.back();
                if(slice.used+sz<=SLICE_MAX_SIZE){
                    dataVector.push_back(data);
                    slice.used+=sz;
                }else{
                    slice.maxData=dataVector.back();
                    dataVector.push_back(data);
                    sliceVector.push_back(SliceIndex(usedSize,sz));
                }
                usedSize+=sz;
            }
            return true;
        }
        int writeToFile(char buf[],int fd,int pageOffset){
            assert(fd>=0);
            exFileSize(fd,pageOffset+BLOCK_MAX_SIZE);
            int pos=0;
            for(auto& e:dataVector){
                e.setToBuffer(buf,pos);
            }
            assert(pos=usedSize);
            //最后一条slice的maxData;
            assert(dataVector.size());
            assert(sliceVector.size());
            auto lastSlice=sliceVector.back();
            lastSlice.maxData=dataVector.back();
            int idxOffset=pos;
            for(auto& e:sliceVector){
                e.setToBuffer(buf,pos);
            }
            toBuffer(buf,pos,idxOffset);
            pwrite(fd,buf,pos,pageOffset);
            return pos;
        }
    };
    struct BlockIndex{
        int beginOffset;
        int usedSize;
        Data minData;
        Data maxData;
        BlockIndex(int offset):beginOffset(offset),usedSize(0),minData(-1,-1,"",""),maxData(-1,-1,"",""){}
        int myByteSize(){ return sizeof(beginOffset)+sizeof(usedSize)+minData.myByteSize()+maxData.myByteSize();}
        void setToBuffer(char buf[],int& pos){
            toBuffer(buf,pos,myByteSize());// 从文件一次读一条需要长度。
            toBuffer(buf,pos,beginOffset);
            toBuffer(buf,pos,usedSize);
            minData.setToBuffer(buf,pos);
            maxData.setToBuffer(buf,pos);
        }
        void getFromBuffer(char buf[],int& pos){
            memcpy(&beginOffset,buf+pos, sizeof(beginOffset));
            pos+= sizeof(beginOffset);
            memcpy(&usedSize,buf+pos, sizeof(usedSize));
            pos+= sizeof(usedSize);
            minData.getFromBuffer(buf,pos);
            maxData.getFromBuffer(buf,pos);
        }
        friend ostream& operator<<(ostream& os,BlockIndex& idx){
            os<<idx.beginOffset<<"\t"<<idx.usedSize<<endl;
            os<<"min"<<endl;
            os<<idx.minData<<endl;
            os<<"max"<<endl;
            os<<idx.maxData<<endl;
        }
    };
    struct Iterator{
        int fd;
        char buffer[BLOCK_MAX_SIZE];
        int pos;
        Data data;
        vector<BlockIndex>::iterator indexIterator;
        int blockDataEnd;
        Iterator(int f):fd(f){}
        void init(){
            // todo 此处存在风险,当indexVector.end()处调用pread内容未知。
            pos=(*indexIterator).usedSize;
            if(pos<=0||pos>4096) return;// 防止走到.end(),发生未知错误。
            pread(fd,buffer,pos,(*indexIterator).beginOffset);
            pos-= sizeof(int);
            memcpy(&blockDataEnd,buffer+pos, sizeof(int));
            pos=0;
        }
        Iterator operator ++(int){
            auto tmp=*this;
            // 未遍历完全
            data.getFromBuffer(buffer,pos);
            if(pos<blockDataEnd){
                ;
            }else{
                indexIterator++;
                init();
                //data.getFromBuffer(buffer,pos);
            }
            return tmp;
        }
        Data operator*(){
            data.getFromBuffer(buffer,pos);
            pos-=data.myByteSize();
            return  data;
        }
        bool operator==(Iterator& b){ return indexIterator==b.indexIterator;}
        bool operator!=(Iterator& b){ return !(*this==b);}
    };

    char _buffer[BLOCK_MAX_SIZE];
    int _pageOffset;
    int _usedFileSize;
//    int _level;
    int _fileDescriptor;
    vector<BlockIndex> _indexVector; // .size()*4096 可以最终确定_indexVector起始地址
    Data _miniData;
    Data _maxData;
    int _indexBeginOffset;
    int _indexEndOffset;
public:
    SSTable(string fileName);
    ~SSTable(){if(_fileDescriptor>=0){close(_fileDescriptor);}}
    void writeBlockIndex();
    void writeFooter();
    int writeSST(SkipList::writeIterator iterator);
    int writeSST(vector<Data>::iterator begin,vector<Data>::iterator end);
    void setMinMax(){
        assert(_indexVector.size());
        _miniData=_indexVector[0].minData;
        _maxData=_indexVector.back().maxData;
    }
    Data findGreatOrEqual(Data& data);
    void readSST();
    void setInfo();//填充sstable文件信息,方便遍历或读取。
    Iterator begin();
    Iterator end();
};

void testSSTable(bool flag);
void testSSTableIterator();
void testSSTableFind();
#endif

#endif //LVDB_SSTABLE_H
