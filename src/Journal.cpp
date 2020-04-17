//
// Created by Administrator on 2020/4/5.
//
#include "Journal.h"
#include "lvmisc.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


Journal::Journal(string filePath){
    _fileDescriptor = open(filePath.c_str(),O_RDWR|O_CREAT);
    if(_fileDescriptor==-1){//error
        showERROR("open file ERROR");
    }
    int rt=chmod(filePath.c_str(),0666);
    if(rt==-1){//error
        showERROR("mode file ERROR");
    }
    struct stat fileStat;
    fstat(_fileDescriptor,&fileStat);
    _logFileSize=fileStat.st_size;
    _lastBlockOffset=(_logFileSize-1)/MAX_BLOCK_SIZE*MAX_BLOCK_SIZE;
    //off_t filePointer=lseek(_fileDescriptor,fileSize,SEEK_SET);
    cout<<_logFileSize<<endl;
//    cout<<_lastBlockOffset<<endl;
    cout<<_blockHeader.logSequence<<"\t"<<_blockHeader.keySequence<<"\t"<<_blockHeader.nextChunkOffset<<endl;
    initBlockerHeader();
    cout<<_blockHeader.logSequence<<"\t"<<_blockHeader.keySequence<<"\t"<<_blockHeader.nextChunkOffset<<endl;
//    cout<<lseek(_fileDescriptor,0,SEEK_CUR)<<endl;
//    cout<<lseek(_fileDescriptor,0,SEEK_END)<<endl;
}

Journal::~Journal(){
    if(_fileDescriptor!=-1){
        //todo 写回block header
        close(_fileDescriptor);
    }
}

void Journal::initBlockerHeader(){
    if(_logFileSize<FILE_ZERO_SIZE){
        _blockHeader.logSequence=time(nullptr);
        _blockHeader.keySequence=_blockHeader.logSequence+rand();
        _blockHeader.nextChunkOffset=0;
    }else{
        pread(_fileDescriptor,&_blockHeader, sizeof(_blockHeader),_lastBlockOffset);
    }
}

void Journal::initData(Data& data,OperationInfo& opInfo,OPERATION_p& op){
    data._sequenceNumber=_blockHeader.keySequence++;
    data._op=op.first;
    data._key=op.second.first;
    data._value=op.second.second;

    opInfo.type=data._op;
    opInfo.keyLength= sizeof(ULL)+ sizeof(char)+ data._key.length();
    opInfo.keyInfo.keySequence=data._sequenceNumber;
    opInfo.keyInfo.type=data._op;
    opInfo.keyInfo.key=data._key;
    opInfo.valueLength=data._value.length();
    opInfo.value=data._value;
}

vector<Journal::LogChunk> Journal::initLogChunk(vector<OperationInfo>&vc){
    vector<LogChunk> res;
    for(auto item:vc){
        int itemSize=item.valueLength+item.keyLength+OperationInfo::OPINFO_HEADER_SIZE;
        if(res.empty()||res.back().logInfo.size+itemSize>LogInfo::LOG_INFO_MAX_SIZE){
            if(res.size()){
                res.back().setCheckSumAndLength();
            }
            res.push_back(LogChunk());
            res.back().logInfo.setLogSequence(_blockHeader.logSequence++);
        }
        LogInfo& last=res.back().logInfo;
        last.operationVector.push_back(item);
        last.count++;
        last.size+=itemSize;
    }
    if(res.size()){
        res.back().setCheckSumAndLength();
    }
    if(res.size()>1){
        res[0].setType(LogChunk::FIRST_CHUNK);
        res[res.size()-1].setType(LogChunk::LAST_CHUNK);
    }else if(res.size()==1){
        res[0].setType(LogChunk::FULL_CHUNK);
    }
    return res;
}
void Journal::_writeLogChunk(Journal::LogChunk& logChunk){
    //todo 写入日志文件
    auto e1=logChunk.logChunkHeader;
    auto e2=logChunk.logInfo;
    cout<<"LogChunk::logChunkHeader"<<endl;
    cout<<e1.checkSum<<"\t"<<e1.length<<"\t"<<e1.type-0<<endl;
    cout<<"LogChunk::logInfo.header"<<endl;
    cout<<e2.logSequence<<"\t"<<e2.count<<"\t"<<e2.size<<endl;
    cout<<"LogChunk::logInfo.OperationVector"<<endl;
    int no=0;
    for(auto e:e2.operationVector){
        cout<<"no "<<no++<<"\t";
        cout<<e.type-0<<"\t"<<e.keyLength<<"\t"<<e.keyInfo.keySequence<<"\t"<<e.keyInfo.type-0<<"\t"<<e.keyInfo.key
        <<"\t"<<e.valueLength<<"\t"<<e.value<<endl;
    }
    char buf[MAX_BLOCK_SIZE];
    int pos=0;

}

Data Journal::_write(OPERATION_p op) {
    Data data;
    OperationInfo opInfo;
    initData(data,opInfo,op);
    cout<<"data: \t##\t";
    cout<<data<<endl;
    cout<<"opInfo: \t##\t";
    cout<<opInfo.type-0<<"\t"<<opInfo.keyLength<<"\t"<<opInfo.keyInfo.keySequence<<"\t"
    << opInfo.keyInfo.type-0<<"\t"<<opInfo.keyInfo.key<<"\t"<<opInfo.valueLength<<"\t"<<opInfo.value<<endl;

    vector<LogChunk> vc=initLogChunk(vector<OperationInfo>()={opInfo});
    for(auto e:vc){
        _writeLogChunk(e);
    }
    return data;
}

bool Journal::_write(vector<OPERATION_p> ops){}
bool Journal::_read(){}

void test_journal(){
    string fpth="/tmp/tmpp/log.txt";
    OPERATION_p opp={1,{"kk","vv"}};
    Journal journal(fpth);
    journal._write(opp);
}