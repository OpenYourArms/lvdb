//
// Created by Administrator on 2020/4/5.
//

#ifndef LVDB_JOURNAL_H
#define LVDB_JOURNAL_H

#ifndef WIN32

#include "lvmisc.h"
#include <iostream>
#include <string>
#include <vector>


using namespace  std;


typedef pair<string,string> KV_p;
typedef pair<int,KV_p> OPERATION_p;

class Journal{
private:
    enum{MAX_BLOCK_SIZE=4096,MAX_LOG_SIZE=4096*1000};
    enum {FILE_ZERO_SIZE = 10};

    struct KeyInfo{
        enum{KEY_INFO_HEAD_SIZE= sizeof(ULL)+ sizeof(char)};
        enum{DELETE=0,PUT=1,GET=2};
        void toBuf(char buf[],int& pos){
            toBuffer(buf,pos,keySequence);
            toBuffer(buf,pos,type);
            toBuffer(buf,pos,key);
        }
        friend ostream& operator<<(ostream& os,KeyInfo& keyInfo){
            os<<"\t\t\t\t\t##########     KeyInfo   Begin     ############"<<endl;
            os<<"KeyInfo.keySequence:\t"<<keyInfo.keySequence<<endl;
            os<<"KeyInfo.type:(0=delete,1=put,2=get)\t"<<keyInfo.type-0<<endl;
            os<<"KeyInfo.key:\t"<<keyInfo.key<<endl;
            os<<"\t\t\t\t\t##########     KeyInfo   End     ############"<<endl;
            return os;
        }
        ULL keySequence;
        char type;
        string key;
    };
    struct OperationInfo{
        enum{OPINFO_HEADER_SIZE= sizeof(char)+sizeof(int)+sizeof(int)};
        enum{DELETE=0,PUT=1,GET=2};
        void toBuf(char buf[],int& pos){
            toBuffer(buf,pos,type);
            toBuffer(buf,pos,keyLength);
            keyInfo.toBuf(buf,pos);
            toBuffer(buf,pos,valueLength);
            toBuffer(buf,pos,value);
        }
        friend ostream& operator<<(ostream& os,OperationInfo& oi){
            os<<"\t\t\t\t\t##########     OperationInfo   Begin     ############"<<endl;
            os<<"OperationInfo.type:\t"<<oi.type-0<<endl;
            os<<"OperationInfo.keyLength\t"<<oi.keyLength<<endl;
            os<<"OperationInfo.KeyInfo"<<endl;
            os<<oi.keyInfo<<endl;
            os<<"OperationInfo.valueLength\t"<<oi.valueLength<<endl;
            os<<"OperationInfo.value\t"<<oi.value<<endl;
            os<<"\t\t\t\t\t##########     OperationInfo   End     ############"<<endl;
            return os;
        }
        char type;
        int keyLength;
        KeyInfo keyInfo;
        int valueLength;
        string value;
    };
    struct LogInfo{
        enum {LOG_INFO_HEADER_SIZE = sizeof(ULL)+sizeof(int)+sizeof(int),LOG_INFO_MAX_SIZE=MAX_BLOCK_SIZE/4*3};
        LogInfo():logSequence(0),count(0),size(LOG_INFO_HEADER_SIZE){}
        void setLogSequence(int seq){logSequence=seq;}
        void toBuf(char buf[],int& pos){
            toBuffer(buf,pos,logSequence);
            toBuffer(buf,pos,count);
            toBuffer(buf,pos,size);
            for(auto& e:operationVector) e.toBuf(buf,pos);
        }
        friend ostream& operator<<(ostream& os,LogInfo& li){
            os<<"\t\t\t\t\t##########     LogInfo   Begin     ############"<<endl;
            os<<"LogInfo.logSequence\t"<<li.logSequence<<endl;
            os<<"LogInfo.count\t"<<li.count<<endl;
            os<<"LogInfo.size\t"<<li.size<<endl;
            os<<"LogInfo.operationVector\tsize:\t"<<li.operationVector.size()<<endl;
            for(int no=0;no<li.operationVector.size();no++){
                cout<<"no "<<no<<"\t";
                os<<li.operationVector[no]<<endl;
            }
            os<<"\t\t\t\t\t##########     LogInfo   End     ############"<<endl;
            return os;
        }
        ULL logSequence;
        int count;
        int size;
        vector<OperationInfo> operationVector;
    };
    struct LogChunk{
        enum{LOG_CHUNK_HEADER_SIZE = sizeof(ULL)+sizeof(int)+sizeof(char)};
        enum{FULL_CHUNK=0,FIRST_CHUNK=1,MIDDLE_CHUNK=2,LAST_CHUNK=3};
        struct Header{
            ULL checkSum;
            int length;
            char type;
            Header():checkSum(0),length(0),type(MIDDLE_CHUNK){}
            void toBuf(char buf[],int& pos){
                toBuffer(buf,pos,checkSum);
                toBuffer(buf,pos,length);
                toBuffer(buf,pos,type);
            }
        };
        //todo 求校验
        void setCheckSumAndLength(){logChunkHeader.length=LOG_CHUNK_HEADER_SIZE+logInfo.size;}
        void setType(char type){logChunkHeader.type=type;}
        void toBuf(char buf[],int& pos){
            logChunkHeader.toBuf(buf,pos);
            logInfo.toBuf(buf,pos);
        }
        friend ostream& operator<<(ostream& os,LogChunk& lc){
            os<<"\t\t\t\t\t##########     LogChunk   Begin     ############"<<endl;
            os<<"LogChunk.logChunkHeader.checkSum\t"<<lc.logChunkHeader.checkSum<<endl;
            os<<"LogChunk.logChunkHeader.length\t"<<lc.logChunkHeader.length<<endl;
            os<<"LogChunk.logChunkHeader.type\t"<<lc.logChunkHeader.type-0<<endl;
            os<<"LogChunk.LogInfo\t"<<endl<<lc.logInfo<<endl;
            os<<"\t\t\t\t\t##########     LogChunk   End     ############"<<endl;
            return os;
        }
        void getChunkData(vector<Data>& vcData);
        Header logChunkHeader;
        LogInfo logInfo;
    };
    struct BlockHeader{
        enum {BLOCK_HEADER_SIZE = sizeof(ULL)+sizeof(ULL)+sizeof(off_t)};
        friend ostream& operator<<(ostream& os,BlockHeader& bh){
            os<<"\t\t\t\t\t##########     BlockHeader   Begin     ############"<<endl;
            os<<"BlockHeader.logSequence\t"<<bh.logSequence<<endl;
            os<<"BlockHeader.keySequence\t"<<bh.keySequence<<endl;
            os<<"BlockHeader.nextChunkOffset\t"<<bh.nextChunkOffset<<endl;
            os<<"\t\t\t\t\t##########     BlockHeader   End     ############"<<endl;
            return os;
        }
        ULL logSequence;
        ULL keySequence;
        off_t nextChunkOffset;
    };
public:
    Journal(string filePath);
    ~Journal();
    void initBlockerHeader();
    void initData(Data& data,OperationInfo& opInfo,OPERATION_p& op);
    vector<LogChunk> initLogChunk(vector<OperationInfo>&vc);
    void _writeLogChunk(LogChunk& logChunk);
    void _writeBlockHeaderBack();
    Data _write(OPERATION_p op);
    vector<Data> _write(vector<OPERATION_p> ops);
    bool findBlockHeader(off_t& block,off_t& bOffset,ULL seq);
    void readKeyInfo(off_t& block,off_t& bOffset,KeyInfo& keyInfo,int length);
    void readOperationInfo(off_t& block,off_t& bOffset,OperationInfo& operationInfo);
    void readLogInfo(off_t& block,off_t& bOffset,LogInfo& logInfo);
    void readLogChunk(off_t& block,off_t& bOffset,LogChunk& logChunk);
    vector<Data> _read(ULL seq);

private:
    BlockHeader _blockHeader;
    int _fileDescriptor;
    off_t _logFileSize;
    off_t _lastBlockOffset;
};

void test_journal();



#endif

#endif //LVDB_JOURNAL_H
