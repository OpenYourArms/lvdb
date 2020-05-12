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
ULL SSTable::writeSST(SkipList::writeIterator iterator) {
    ULL mxSeq=0;
    unique_ptr<BlockStruct> bp = nullptr;
    assert(_pageOffset==0);
    while(iterator.nowPos!=nullptr) {
        Data& data=iterator.nowPos->_data;
        mxSeq=max(mxSeq,data._sequenceNumber);
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
    return mxSeq;
}
ULL SSTable::writeSST(vector<Data>::iterator begin, vector<Data>::iterator end) {
    ULL mxSeq=0;
    unique_ptr<BlockStruct> bp = nullptr;
    assert(_pageOffset==0);
    while(begin!=end) {
        Data& data=*begin;
        mxSeq=max(mxSeq,data._sequenceNumber);
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
        begin++;
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
    return mxSeq;
}
Data SSTable::findGreatOrEqual(Data &data) {
    // 前提，sst的文件描述符读到，setInfo也调用了。
    /*
     * 在索引信息中，先二分，查找范围，确定页，再进行页内查找；
     * 在页内部，先遍历索引信息，再细化搜索
     * */
    // 二分
    int l=0,r=_indexVector.size();
    int mi=0;
    while(l<r){
        mi=l+(r-l)/2;
        // 两种情况，一是minData大于等于data,二是minData<data<=maxData,所以合并变成maxData大于等于data;
        if(!Data::compare(_indexVector[mi].maxData,data)){
            r=mi;
        }else{
            l=mi+1;
        }
    }
    //确认 minData <= data <= maxData
    bool inMiddle=!Data::compare(_indexVector[r].maxData,data)&&!Data::compare(data,_indexVector[r].minData);
    // 不在此页，返回无效值
    if(!inMiddle){
        return Data::getInvalidData();
    }
    // 页内索引确定
    auto& page=_indexVector[r];
    pread(_fileDescriptor,_buffer,page.usedSize,page.beginOffset);
    BlockStruct bs;
    int sliceNum=0;
    // slice
    int pos=page.usedSize;
    pos-= sizeof(int);
    int idxOff=0;
    memcpy(&idxOff,_buffer+pos, sizeof(int));
    while(idxOff<pos){
        bs.sliceVector.push_back(BlockStruct::SliceIndex(-1,-1));
        auto& si=bs.sliceVector.back();
        si.getFromBuffer(_buffer,idxOff);
        if(Data::compare(si.maxData,data)){
            sliceNum++;
        }
    }
    assert(sliceNum>=0&&sliceNum<bs.sliceVector.size());
    BlockStruct::SliceIndex okSlice=bs.sliceVector[sliceNum];
    // 页内段查找
    int be=okSlice.sliceOffset;
    int tar=be+okSlice.used;
    while(be<tar){
        bs.dataVector.push_back(Data());
        auto& dt=bs.dataVector.back();
        dt.getFromBuffer(_buffer,be);
        bs.usedSize+=dt.myByteSize();
        if(!Data::compare(dt,data)){
            return dt;
        }
    }
    return Data::getInvalidData();
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
void SSTable::setInfo() {
    struct stat fileStat;
    fstat(_fileDescriptor,&fileStat);
    int pos=fileStat.st_size;
    assert(pos> sizeof(int)+ sizeof(int));
    pos-= sizeof(_indexBeginOffset)+ sizeof(_indexEndOffset);
    pread(_fileDescriptor,&_indexBeginOffset, sizeof(_indexBeginOffset),pos);
    pos+= sizeof(_indexBeginOffset);
    pread(_fileDescriptor,&_indexEndOffset, sizeof(_indexEndOffset),pos);
    //read blockIndex
    pos=_indexBeginOffset;
    int len=0;
    while(pos<_indexEndOffset){
        pread(_fileDescriptor,&len, sizeof(len),pos);
        pos+=sizeof(len);
        pread(_fileDescriptor,_buffer,len,pos);
        pos+=len;
        len=0;
        _indexVector.push_back(BlockIndex(-1));
        auto& e=_indexVector.back();
        e.getFromBuffer(_buffer,len);
    }
    _usedFileSize=fileStat.st_size;
    _miniData=_indexVector.front().minData;
    _maxData=_indexVector.back().maxData;
}
SSTable::Iterator SSTable::begin() {
    Iterator iterator(_fileDescriptor);
    iterator.indexIterator=_indexVector.begin();
    iterator.init();
    return iterator;
}
SSTable::Iterator SSTable::end() {
    Iterator iterator(_fileDescriptor);
    iterator.indexIterator=_indexVector.end();
    return iterator;
}

void testSSTable(bool flag){
    string file="/tmp/tmpp/SST0001.db";
    SSTable ssTable(file);
    SkipList sl;
    vector<int> vc;
    //vc={1,3,8,6,2,4,9,7,5,19,22,14};
    int N=1500;
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
/*
testSSTable(1);
testSSTable(0);
testSSTableIterator(); 三个配合测试效果不错
 * */
void testSSTableIterator(){
    string file="/tmp/tmpp/SST0001.db";
    SSTable ssTable(file);
    ssTable.setInfo();
    auto e=ssTable.begin();
    auto end=ssTable.end();
    while(e!=end){
        auto data=*e;
        if(data._sequenceNumber==1171){
            int iqy=5;
            iqy++;
        }
        cout<<data<<endl;
        e++;
    }
}
void testSSTableFind(){
    string file="/tmp/tmpp/SST17db";
    SSTable ssTable(file);
    ssTable.setInfo();
    Data targetData(5787,1,"5787","");
    auto rt=ssTable.findGreatOrEqual(targetData);
    cout<<"Find is :\n"<<rt<<endl;
}

