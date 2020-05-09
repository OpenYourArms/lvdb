//
// Created by Administrator on 2020/4/24.
//

#ifndef LVDB_SSTMANAGE_H
#define LVDB_SSTMANAGE_H

#ifndef WIN32

#include "lvmisc.h"
#include "SkipList.h"
#include <string>
#include <cassert>
#include <fcntl.h>
#include <vector>

using namespace std;
class SSTManage{
    // 2m*8, 20m*4, 200m*8, 2000m*16, 20G*32,
    enum{MAX_LEVEL=4,MAX_BUFFER_SIZE=1024};
    enum{LEVEL0_SIZE=1*1024*1024,LEVEL0_FILE_COUNT=4};// 1M bytes
    // todo 为了测试，把SSTRecord 开放
public:
    struct SSTRecord{
        int levelNumber;
        int fileSize;
        Data minData;
        Data maxData;
        string fileName;
        //int useCount;先不要，做成单线程
        //SSTRecord():levelNumber(0),fileSize(0),fileName(0)/*,useCount(0)*/{}
        void setToBuffer(char buf[],int& pos){
            toBuffer(buf,pos,levelNumber);
            toBuffer(buf,pos,fileSize);
            minData.setToBuffer(buf,pos);
            maxData.setToBuffer(buf,pos);
            toBuffer(buf,pos,fileName);
        }
        void getFromBuffer(char buf[],int& pos){
            memcpy(&levelNumber,buf+pos, sizeof(levelNumber));
            pos+= sizeof(levelNumber);
            memcpy(&fileSize,buf+pos, sizeof(fileSize));
            pos+= sizeof(fileSize);
            minData.getFromBuffer(buf,pos);
            maxData.getFromBuffer(buf,pos);
            fileName=string(buf+pos);
        }
        int myBufferSize(){
            return sizeof(levelNumber)+sizeof(fileSize)+minData.myByteSize()+maxData.myByteSize()+strlen(fileName.c_str())+1;
        }
        friend ostream& operator<<(ostream& os,SSTRecord& record){
            os<<"fileName:\t"<<record.fileName<<"\t\tlevel:\t"<<record.levelNumber<<"\t\tfileSize:\t"<<record.fileSize<<endl;
            os<<"\t\tminData:"<<endl;
            os<<record.minData<<endl;
            os<<"\t\tmaxData:"<<endl;
            os<<record.maxData<<endl;
            return os;
        }
    };
    // todo 为了测试放开
public:
    char _buffer[MAX_BUFFER_SIZE];
    int _fileDescriptor;
    int _fileSize;
    int _counter;
    int _nextNumber;
    vector<int> _levelSizeVector;
    vector<vector<SSTRecord>> SSTInfoVector;
public:
    void loadSSTInfo();
    void storeSSTInfo();
    explicit SSTManage(string filePath):_counter(0),_nextNumber(0),_fileDescriptor(-1),_levelSizeVector(MAX_LEVEL,0),SSTInfoVector(MAX_LEVEL,vector<SSTRecord>()){
        assert(filePath.length()>0);
        _fileDescriptor=open(filePath.c_str(),O_RDWR|O_CREAT);
        if(_fileDescriptor==-1){
            showERROR("SSTManage Open File ERROR!");
        }
        int rt=chmod(filePath.c_str(),0666);
        if(rt==-1){//error
            showERROR("mode file ERROR");
        }
        struct stat fileStat;
        fstat(_fileDescriptor,&fileStat);
        _fileSize= sizeof(_counter)+ sizeof(_nextNumber);
        if(fileStat.st_size> sizeof(_counter)+ sizeof(_nextNumber)){
            loadSSTInfo();
        } else{
            _counter=0;
            _nextNumber=0;
        }
    }
    ~SSTManage(){
        if(_fileDescriptor>=0){
            storeSSTInfo();
            close(_fileDescriptor);
        }
    }
    void writeToSST(SkipList::writeIterator iterator);
    void writeToSST(vector<Data>::iterator begin,vector<Data>::iterator end,int lev);
    // todo 合并多个文件，传入迭代器SST迭代器。
    vector<SSTRecord> choseFileRecord(int level);
    vector<Data> mergeData(vector<SSTRecord>& files);
    void mergeSSTable(int lev);
};

void test_SSTManage_read_write();

void test_SSTManage_with_SSTable();

void test_SSTManage_SSTable_Merge();

#endif

#endif //LVDB_SSTMANAGE_H
