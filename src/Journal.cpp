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
    cout<<"logFileName\t"<<filePath<<endl;
    cout<<"logFileSize\t"<<_logFileSize<<endl;
    cout<<_blockHeader<<endl;
    initBlockerHeader();
    cout<<_blockHeader<<endl;
//    cout<<lseek(_fileDescriptor,0,SEEK_CUR)<<endl;
//    cout<<lseek(_fileDescriptor,0,SEEK_END)<<endl;
}

Journal::~Journal(){
    if(_fileDescriptor!=-1){
        _writeBlockHeaderBack();
        close(_fileDescriptor);
    }
}

void Journal::initBlockerHeader(){
    if(_logFileSize<FILE_ZERO_SIZE){
        _blockHeader.logSequence=time(nullptr);
        _blockHeader.keySequence=_blockHeader.logSequence+rand();
        _blockHeader.nextChunkOffset=sizeof(_blockHeader);
        exFileSize(_fileDescriptor,MAX_BLOCK_SIZE);
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
    opInfo.keyLength= sizeof(ULL)+ sizeof(char)+ strlen(data._key.c_str())+1;
    opInfo.keyInfo.keySequence=data._sequenceNumber;
    opInfo.keyInfo.type=data._op;
    opInfo.keyInfo.key=data._key;
    opInfo.valueLength=strlen(data._value.c_str())+1;
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
    cout<<logChunk<<endl;
    char buf[MAX_BLOCK_SIZE];
    int pos=0;
    logChunk.toBuf(buf,pos);
    cout<<"logChunk write done:\t"<<pos<<endl;
    cout<<buf<<endl;
    int leftSize=MAX_BLOCK_SIZE-_blockHeader.nextChunkOffset;
    if(leftSize<logChunk.logChunkHeader.length){
        _writeBlockHeaderBack();
        _blockHeader.nextChunkOffset= sizeof(_blockHeader);
        _lastBlockOffset+=MAX_BLOCK_SIZE;
        exFileSize(_fileDescriptor,_lastBlockOffset+MAX_BLOCK_SIZE);
    }
    pwrite(_fileDescriptor,buf,pos,_lastBlockOffset+_blockHeader.nextChunkOffset);
    _blockHeader.nextChunkOffset+=pos;
}

void Journal::_writeBlockHeaderBack(){
    pwrite(_fileDescriptor,&_blockHeader, sizeof(_blockHeader),_lastBlockOffset);
}

Data Journal::_write(OPERATION_p op) {
    Data data;
    OperationInfo opInfo;
    initData(data,opInfo,op);
    cout<<"data: \t##\t";
    cout<<data<<endl;
    cout<<opInfo<<endl;
    vector<LogChunk> vc=initLogChunk(vector<OperationInfo>()={opInfo});
    for(auto e:vc){
        _writeLogChunk(e);
    }
    return data;
}

vector<Data> Journal::_write(vector<OPERATION_p> ops){
    vector<Data> res;
    vector<OperationInfo> opInfoVc;
    for(auto& e:ops){
        res.push_back(Data());
        opInfoVc.push_back(OperationInfo());
        initData(res.back(),opInfoVc.back(),e);
    }
    vector<LogChunk> vc=initLogChunk(opInfoVc);
    for(auto& e:vc){
        _writeLogChunk(e);
    }
    return res;
}

bool Journal::findBlockHeader(off_t& block,off_t& bOffset,ULL seq){
    int lo=0,hi=_lastBlockOffset/MAX_BLOCK_SIZE;
    while(lo<hi){
        int mi=lo+(hi-lo)/2;
        BlockHeader bh;
        pread(_fileDescriptor,&bh, sizeof(BlockHeader),mi*MAX_BLOCK_SIZE);
        if(bh.keySequence>=seq){
            hi=mi;
        }else{
            lo=mi+1;
        }
    }
    block=lo*MAX_BLOCK_SIZE;
    bOffset= sizeof(BlockHeader);
    ULL s=0;
    int chunkSize=0;
    do{
        LogChunk logChunk;
        readLogChunk(block,bOffset,logChunk);
        chunkSize=logChunk.logChunkHeader.length;
        vector<Data> dataVc;
        logChunk.getChunkData(dataVc);
        s=dataVc.back()._sequenceNumber;
    }while(s<seq);
    bOffset-=chunkSize;
    // todo 返回值没有暂时，需要的话还要遍历data数组。
    return s==seq;
}
void Journal::readKeyInfo(off_t& block,off_t& bOffset,KeyInfo& keyInfo,int length){
    //todo
    int sz= sizeof(keyInfo.keySequence);
    pread(_fileDescriptor,&keyInfo.keySequence,sz,block+bOffset);
    bOffset+=sz;
    sz= sizeof(keyInfo.type);
    pread(_fileDescriptor,&keyInfo.type,sz,block+bOffset);
    bOffset+=sz;
    sz= length-keyInfo.KEY_INFO_HEAD_SIZE;
    char buf[MAX_LOG_SIZE];
    pread(_fileDescriptor,buf,sz,block+bOffset);
    bOffset+=sz;
    keyInfo.key=buf;
}
void Journal::readOperationInfo(off_t& block,off_t& bOffset,OperationInfo& operationInfo){
    //todo 优化pread,先写到内存，再用memcpy函数。
    int sz= sizeof(operationInfo.type);
    pread(_fileDescriptor,&operationInfo.type,sz,block+bOffset);
    bOffset+=sz;
    sz= sizeof(operationInfo.keyLength);
    pread(_fileDescriptor,&operationInfo.keyLength,sz,block+bOffset);
    bOffset+=sz;
    readKeyInfo(block,bOffset,operationInfo.keyInfo,operationInfo.keyLength);
    sz= sizeof(operationInfo.valueLength);
    pread(_fileDescriptor,&operationInfo.valueLength,sz,block+bOffset);
    bOffset+=sz;
    sz=operationInfo.valueLength;
    char buf[MAX_LOG_SIZE];
    pread(_fileDescriptor,buf,sz,block+bOffset);
    bOffset+=sz;
    operationInfo.value=buf;
}
void Journal::readLogInfo(off_t& block,off_t& bOffset,LogInfo& logInfo){
    //todo
    int sz= sizeof(logInfo.logSequence);
    pread(_fileDescriptor,&logInfo.logSequence,sz,block+bOffset);
    bOffset+=sz;
    sz= sizeof(logInfo.count);
    pread(_fileDescriptor,&logInfo.count,sz,block+bOffset);
    bOffset+=sz;
    sz= sizeof(logInfo.size);
    pread(_fileDescriptor,&logInfo.size,sz,block+bOffset);
    bOffset+=sz;
    sz=logInfo.size-LogInfo::LOG_INFO_HEADER_SIZE;
    while(sz){
        logInfo.operationVector.push_back(OperationInfo());
        auto& e=logInfo.operationVector.back();
        readOperationInfo(block,bOffset,e);
        sz-=e.keyLength+e.valueLength+OperationInfo::OPINFO_HEADER_SIZE;
    }
}
void Journal::readLogChunk(off_t& block,off_t& bOffset,LogChunk& logChunk){
    int sz=sizeof(logChunk.logChunkHeader.checkSum);
    pread(_fileDescriptor,&logChunk.logChunkHeader.checkSum,sz,block+bOffset);
    bOffset+= sz;
    sz=sizeof(logChunk.logChunkHeader.length);
    pread(_fileDescriptor,&logChunk.logChunkHeader.length,sz,block+bOffset);
    bOffset+=sz;
    sz= sizeof(logChunk.logChunkHeader.type);
    pread(_fileDescriptor,&logChunk.logChunkHeader.type,sz,block+bOffset);
    bOffset+=sz;
    //sz= logChunk.logChunkHeader.length-LogChunk::LOG_CHUNK_HEADER_SIZE; logInfo有自己的size了。
    readLogInfo(block,bOffset,logChunk.logInfo);
    // todo 读完本块最后一个chunk,需要自动跳block,方案有二，1 - 重新读header; 2 - 建立索引存每个block的尺寸。
    // todo 可以被pread优化时把block放入内存，就可以直接取到了。
    // 以上todo已被解决，换块操作放在 _read函数的nowHeader 相关操作。
}
void Journal::LogChunk::getChunkData(vector<Data> &vcData){
    //todo
    for(auto& e:logInfo.operationVector){
        vcData.push_back(Data());
        auto& f=vcData.back();
        f._key=e.keyInfo.key;
        f._value=e.value;
        f._sequenceNumber=e.keyInfo.keySequence;
        f._op=e.keyInfo.type;
    }
}
vector<Data> Journal::_read(ULL seq){
    vector<Data> res;
    off_t bOffset;
    off_t block=0;
    bool flag=findBlockHeader(block,bOffset,seq);
    BlockHeader nowHeader;
    pread(_fileDescriptor,&nowHeader, sizeof(nowHeader),block);
    while(!(block==_lastBlockOffset&&bOffset==_blockHeader.nextChunkOffset)){
        LogChunk logChunk;
        // 读到结尾，换下一块
        if(bOffset==nowHeader.nextChunkOffset){
            block+=MAX_BLOCK_SIZE;
            bOffset= sizeof(nowHeader);
            pread(_fileDescriptor,&nowHeader, sizeof(nowHeader),block);
        }
        readLogChunk(block,bOffset,logChunk);
        logChunk.getChunkData(res);
//        cout<<"read journal:\t"<<res.back()<<"\tblock:\t"<<block<<"\tbOffset:\t"<<bOffset<<endl;
    }
    return res;
}

void test_journal(){
    string fpth="/tmp/tmpp/log.txt";
    OPERATION_p opp={1,{"kk","vv"}};
    vector<OPERATION_p> opvc;
    int caseNum=20;
    for(int i=0;i<caseNum;i++){
        int op1=rand()%3;
        string key="",item=to_string(rand());
        int ti=rand()%2+1;
        for(int j=0;j<ti;j++){
            key+=item;
        }
        cout<<key<<endl;
        opvc.push_back({op1,{key,key}});
    }
    Journal journal(fpth);
//    Journal::LogChunk logChunk;

//    off_t block=0,bOffset= sizeof(Journal::BlockHeader);
//    journal.readLogChunk(block,bOffset,logChunk);
//    cout<<logChunk<<endl;

//    bool flag=journal.findBlockHeader(block,bOffset,2307195330);
//    cout<<"res.bool\t"<<flag<<endl;
//    cout<<block<<"\t"<<bOffset<<endl;
    auto vcd=journal._write(opvc);
    for(auto d:vcd) cout<<d<<endl;
    cout<<"\t\t\t\t\t\t\tDone!"<<endl;
    auto e=journal._read(0);
    for(auto f:e) cout<<f<<endl;
//    for(auto e:opvc){
//        journal._write(e);
//    }
    //journal._write(opvc);
}