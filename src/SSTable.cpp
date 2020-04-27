//
// Created by Administrator on 2020/4/23.
//

#include <memory>
#include "SSTable.h"

SSTable::SSTable(string filePath):_pageOffset(0),_miniData(-1,-1,"",""),_maxData(-1,-1,"",""),_indexBeginOffset(0),_indexEndOffset(0){
    assert(filePath.size()>1);
    _usedFileSize=0;
    _fileDescriptor=open(filePath.c_str(),O_RDWR|O_CREAT);
    if(_fileDescriptor==-1){
        showERROR("SSTManage Open File ERROR!");
    }
    int rt=chmod(filePath.c_str(),0666);
    if(rt==-1){//error
        showERROR("mode file ERROR");
    }
}
void SSTable::writeBlockIndex(){
    int sz=0;
    for(auto& e:_indexVector){
        sz+= sizeof(e.myByteSize());
        sz+=e.myByteSize();
    }
    exFileSize(_fileDescriptor,_pageOffset+sz);
    for(auto&e:_indexVector){
        int pos=0;
        e.setToBuffer(_buffer,pos);
        pwrite(_fileDescriptor,_buffer,pos,_pageOffset);
        _pageOffset+=pos;
    }
    _indexEndOffset=_pageOffset;
}
void SSTable::writeFooter() {
    int sz= sizeof(_indexBeginOffset)+ sizeof(_indexEndOffset);
    exFileSize(_fileDescriptor,_pageOffset+sz);
    int pos=0;
    toBuffer(_buffer,pos,_indexBeginOffset);
    toBuffer(_buffer,pos,_indexEndOffset);
    pwrite(_fileDescriptor,_buffer,pos,_pageOffset);
    _pageOffset+=sz;
}
int SSTable::writeSST(SkipList::writeIterator iterator) {
    unique_ptr<BlockStruct> bp = nullptr;
    assert(_pageOffset==0);
    while(iterator.nowPos!=nullptr) {
        Data& data=iterator.nowPos->_data;
        if(!bp){
            bp.reset(new BlockStruct());
            _indexVector.push_back(BlockIndex(_pageOffset));
            auto& idx=_indexVector.back();
            bp->addData(data);
            idx.minData=data;
        }else{
            auto& lastIdx=_indexVector.back();
            if(bp->addData(data)){
                ;//lastIdx.usedSize+=data.myByteSize();
            }else{
                lastIdx.maxData=bp->dataVector.back();
                lastIdx.usedSize=bp->writeToFile(_buffer,_fileDescriptor,_pageOffset);
                BlockStruct* p=bp.release();
                assert(p);
                delete p;
                bp.reset(new BlockStruct());
                _pageOffset+=BLOCK_MAX_SIZE;
                _indexVector.push_back(BlockIndex(_pageOffset));
                auto& idx=_indexVector.back();
                bp->addData(data);
                idx.minData=data;
            }
        }
        iterator++;
    }
    if(bp){
        auto& lastIdx=_indexVector.back();
        lastIdx.maxData=bp->dataVector.back();
        lastIdx.usedSize=bp->writeToFile(_buffer,_fileDescriptor,_pageOffset);
        BlockStruct* p=bp.release();
        assert(p);
        delete p;
        _pageOffset+=BLOCK_MAX_SIZE;
    }
    _indexBeginOffset=_pageOffset;
    writeBlockIndex();
    _indexEndOffset=_pageOffset;
    writeFooter();
    _usedFileSize=_pageOffset;
    close(_fileDescriptor);
    _fileDescriptor=-2;
}
void SSTable::readSST() {
    assert(_fileDescriptor>=0);
    struct stat fileStat;
    fstat(_fileDescriptor,&fileStat);
    int pos=fileStat.st_size;
    pos-= sizeof(_indexBeginOffset)+ sizeof(_indexEndOffset);
    pread(_fileDescriptor,&_indexBeginOffset, sizeof(_indexBeginOffset),pos);
    pos+= sizeof(_indexBeginOffset);
    pread(_fileDescriptor,&_indexEndOffset, sizeof(_indexEndOffset),pos);
    //read blockIndex
    cout<<"### blockIndex Info:\t"<<_indexBeginOffset<<"\t\t--\t\t"<<_indexEndOffset<<endl;
    pos=_indexBeginOffset;
    int len=0;
    cout<<"### blockIndex Vector\t:"<<endl;
    while(pos<_indexEndOffset){
        pread(_fileDescriptor,&len, sizeof(len),pos);
        pos+=sizeof(len);
        pread(_fileDescriptor,_buffer,len,pos);
        pos+=len;
        len=0;
        _indexVector.push_back(BlockIndex(-1));
        auto& e=_indexVector.back();
        e.getFromBuffer(_buffer,len);
        cout<<e<<endl;
        cout<<"\t-----------------------------------------------------------\t"<<endl;
    }
    int pgNumber=0;
    for(auto& page:_indexVector){
        cout<<"Page:************************\t\t"<<pgNumber++<<endl;
        pread(_fileDescriptor,_buffer,page.usedSize,page.beginOffset);
        BlockStruct bs;
        // slice
        int pos=page.usedSize;
        pos-= sizeof(int);
        int idxOff=0;
        memcpy(&idxOff,_buffer+pos, sizeof(int));
        while(idxOff<pos){
            bs.sliceVector.push_back(BlockStruct::SliceIndex(-1,-1));
            auto& si=bs.sliceVector.back();
            si.getFromBuffer(_buffer,idxOff);
        }
        int sliceNumber=0,dataNumber=0;
        for(auto& slice:bs.sliceVector){
            cout<<"slice:\t\t"<<sliceNumber++<<endl;
            int be=slice.sliceOffset;
            int tar=be+slice.used;
            while(be<tar){
                bs.dataVector.push_back(Data());
                auto& data=bs.dataVector.back();
                data.getFromBuffer(_buffer,be);
                bs.usedSize+=data.myByteSize();
                cout<<"data \t"<<dataNumber++<<"\t"<<endl;
                cout<<data<<endl;
            }
        }
    }
    close(_fileDescriptor);
    _fileDescriptor=-2;
}


void testSSTable(bool flag){
    string file="/tmp/tmpp/SST0001.db";
    SSTable ssTable(file);
    SkipList sl;
    vector<int> vc;
    //vc={1,3,8,6,2,4,9,7,5,19,22,14};
    int N=100000;
    for(int i=0;i<N;i++){
        vc.push_back(i);
    }
    for(auto e:vc){
        string k=to_string(e);
        string v=k;
        Data data(e,Data::PUT,k,v);
        sl.insert(data);
    }
    if(flag){
        ssTable.writeSST(sl.getIterator());
    } else{
        ssTable.readSST();
    }
}

