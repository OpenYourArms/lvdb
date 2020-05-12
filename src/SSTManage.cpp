//
// Created by Administrator on 2020/4/24.
//

#include "SSTManage.h"
#include "SSTable.h"
#include <set>
#include <queue>
#include <memory>

void SSTManage::loadSSTInfo(){
    //counter
    int pos=0;
    pread(_fileDescriptor,&_counter, sizeof(_counter),pos);
    pos+= sizeof(_counter);
    // nextNumber
    pread(_fileDescriptor,&_nextNumber, sizeof(_nextNumber),pos);
    pos+= sizeof(_nextNumber);
    // records
    int length=0,bufPos=0;
    for(int i=0;i<_counter;i++){
        // length
        pread(_fileDescriptor,&length, sizeof(length),pos);
        pos+= sizeof(length);
        // buffer
        pread(_fileDescriptor,_buffer,length,pos);
        pos+=length;
        SSTRecord record;
        bufPos=0;
        record.getFromBuffer(_buffer,bufPos);
        SSTInfoVector[record.levelNumber].push_back(record);
        _levelSizeVector[record.levelNumber]+=record.fileSize;
    }
    _fileSize=pos;
}

void SSTManage::storeSSTInfo() {
    exFileSize(_fileDescriptor,_fileSize);
    int pos=0;
    // counter
    pwrite(_fileDescriptor,&_counter, sizeof(_counter),pos);
    pos+= sizeof(_counter);
    // nextNumber
    pwrite(_fileDescriptor,&_nextNumber, sizeof(_nextNumber),pos);
    pos+= sizeof(_nextNumber);
    // record
    int rsz=0;int ml=0;
    cout<<endl;
    for(auto& r:SSTInfoVector){
        cout<<ml++<<"::\t";
        for(auto& c:r){
            int l=0;
            rsz=c.myBufferSize();
            toBuffer(_buffer,l,rsz);
            c.setToBuffer(_buffer,l);
            pwrite(_fileDescriptor,_buffer,l,pos);
            pos+=l;
            cout<<c.fileName<<"\t";
        }
        cout<<endl;
    }
}
void SSTManage::writeToSST(SkipList::writeIterator iterator){
    SSTRecord record;
    record.levelNumber=0;
    record.fileName="/tmp/tmpp/SST"+to_string(_nextNumber++)+"db";
    SSTable table(record.fileName);
    table.writeSST(iterator);
    table.setMinMax();
    record.minData=table._miniData;
    record.maxData=table._maxData;
    record.fileSize=table._usedFileSize;
    _levelSizeVector[record.levelNumber]+=record.fileSize;
    SSTInfoVector[record.levelNumber].push_back(record);
    _counter++;
    _fileSize+=record.myBufferSize();
    // 超过限制，进行SST合并
    if( _levelSizeVector[record.levelNumber]>LEVEL0_SIZE||SSTInfoVector[record.levelNumber].size()>LEVEL0_FILE_COUNT){
        mergeSSTable(0);
    }
}
void SSTManage::writeToSST(vector<Data>::iterator begin, vector<Data>::iterator end,int lev=0) {
    SSTRecord record;
    record.levelNumber=lev;
    record.fileName="/tmp/tmpp/SST"+to_string(_nextNumber++)+"db";
    SSTable table(record.fileName);
    table.writeSST(begin,end);
    table.setMinMax();
    record.minData=table._miniData;
    record.maxData=table._maxData;
    record.fileSize=table._usedFileSize;
    _levelSizeVector[record.levelNumber]+=record.fileSize;
    SSTInfoVector[record.levelNumber].push_back(record);
    _counter++;
    _fileSize+=record.myBufferSize();
    int sz=LEVEL0_SIZE;
    int t=lev;
    while(t--){
        sz*=10;
    }
    if(lev+1<MAX_LEVEL&&_levelSizeVector[record.levelNumber]>sz){
        mergeSSTable(lev);
    }
}
vector<SSTManage::SSTRecord> SSTManage::choseFileRecord(int level) {
    vector<SSTRecord> res;
    assert(level>=0&&level<MAX_LEVEL);
    assert(SSTInfoVector[level].size()>0);
    int modd=SSTInfoVector[level].size();
    int chose=rand()%modd;// 人家用的是轮转选择
    res.push_back(SSTInfoVector[level][chose]);
    Data& mindata=res[0].minData;
    Data& maxdata=res[0].maxData;
    set<string> chosedSet;
    chosedSet.insert(res[0].fileName);
    if(level==0){// 本层相交文件
        for(auto& e:SSTInfoVector[level]){
            if(chosedSet.count(e.fileName)!=0) continue;
            if(Data::compare(e.maxData,mindata)||Data::compare(maxdata,e.minData)){
                continue;
            }else{
                res.push_back(e);
                chosedSet.insert(e.fileName);
                mindata=Data::compare(e.minData,mindata)?e.minData:mindata;
                maxdata=Data::compare(maxdata,e.maxData)?e.maxData:maxdata;
            }
        }
    }
    if(level<MAX_LEVEL&&SSTInfoVector[level+1].size()){ //下一层相交文件
        for(auto& e:SSTInfoVector[level+1]){
            if(Data::compare(e.maxData,mindata)||Data::compare(maxdata,e.minData)){
                continue;
            }else{
                res.push_back(e);
                mindata=Data::compare(e.minData,mindata)?e.minData:mindata;
                maxdata=Data::compare(maxdata,e.maxData)?e.maxData:maxdata;
            }
        }
    }
    for(auto& e:SSTInfoVector[level]){// 本层回炉一下，尽量扩大文件范围
        if(chosedSet.count(e.fileName)!=0) continue;
        if(Data::compare(e.maxData,mindata)||Data::compare(maxdata,e.minData)){
            continue;
        }else{
            res.push_back(e);
            chosedSet.insert(e.fileName);
            mindata=Data::compare(e.minData,mindata)?e.minData:mindata;
            maxdata=Data::compare(maxdata,e.maxData)?e.maxData:maxdata;
        }
    }
    return res;
}
vector<Data> SSTManage::mergeData(vector<SSTRecord> &files) {
    vector<unique_ptr<SSTable>> SSTVector;
    vector<SSTable::Iterator> itVector;
    vector<SSTable::Iterator> itEndVector;
    for(auto&e :files){
        SSTVector.push_back(unique_ptr<SSTable>(new SSTable(e.fileName)));// 在尾部new 一个 SSTable给智能指针.
        (*SSTVector.back()).setInfo();// SST仅仅是打开文件
        itVector.push_back((*SSTVector.back()).begin());
        itEndVector.push_back((*SSTVector.back()).end());
    }
    vector<Data> dataVector;
    // 合并
    typedef pair<SSTable::Iterator,int> itPair;// 迭代器，对应的vector下标
    auto cmpp=[](itPair p1,itPair p2){
        Data data1=*(p1.first);
        Data data2=*(p2.first);
        return !Data::compare(data1,data2);
    };
    priority_queue<itPair,vector<itPair>, decltype(cmpp)> pQ(cmpp);
    for(int i=0;i<itVector.size();i++){
        pQ.push({itVector[i],i});
    }
    while(pQ.size()){
        auto e=pQ.top();
        pQ.pop();
        auto end=itEndVector[e.second];
        dataVector.push_back(*e.first);
        e.first++;
        if(e.first!=end){
            pQ.push(e);
        }
    }
    return dataVector;
}
void SSTManage::mergeSSTable(int lev) {
    vector<SSTRecord> recordVc=choseFileRecord(lev);
    set<string> saw;
    for(auto& e:recordVc){
        saw.insert(e.fileName);
    }
    vector<Data> dataVc=mergeData(recordVc);
    // 写到lev+1层
    writeToSST(dataVc.begin(),dataVc.end(),lev+1);
    // 清除无效record,删除文件（暂时不删）；
    vector<SSTRecord> allRecord;
    for(auto& r:SSTInfoVector){
        for(auto& c:r){
            if(saw.count(c.fileName)==0) allRecord.push_back(c);
        }
    }
    _fileSize= sizeof(_counter)+ sizeof(_nextNumber);
    _counter=0;
    /*
     * _levelSizeVector(MAX_LEVEL,0),SSTInfoVector(MAX_LEVEL,vector<SSTRecord>())
     * */
    _levelSizeVector=vector<int>(MAX_LEVEL,0);
    SSTInfoVector=vector<vector<SSTRecord>>(MAX_LEVEL,vector<SSTRecord>());
    for(auto& e:allRecord){
        _levelSizeVector[e.levelNumber]+=e.fileSize;
        SSTInfoVector[e.levelNumber].push_back(e);
        _counter++;
        _fileSize+=e.myBufferSize();
    }
    // 删除文件
    for(auto e:saw){
        remove(e.c_str());
    }
}
void test_SSTManage_read_write(){
    SSTManage sstM("/tmp/tmpp/SSTManage");
    vector<SSTManage::SSTRecord> vc;
    for(auto& r:sstM.SSTInfoVector){
        for(auto& c:r){
            vc.push_back(c);
        }
    }
    for(auto e:vc){
        cout<<e<<endl;
    }
    cout<<"\t\t\t\t\t\t\t####################"<<endl;
    int n=2;
    int oldc=sstM._counter;
    for(int i=vc.size();i<oldc+n;i++){
        vc.push_back(SSTManage::SSTRecord());
        auto& e=vc.back();
        e.levelNumber=rand()%12;
        e.fileSize=(rand()%10)*100+20*87;
        e.minData=Data(i*2+1,rand()%3,to_string(i),to_string(i*12));
        e.maxData=Data(i*2+2,rand()%3,to_string(i+10086),to_string((i+105)*12));
        e.fileName=to_string(sstM._nextNumber++);
        sstM.SSTInfoVector[e.levelNumber].push_back(e);
        sstM._levelSizeVector[e.levelNumber]+=e.fileSize;
        sstM._counter++;
        sstM._fileSize+=e.myBufferSize();
        cout<<endl<<endl;
        cout<<e<<endl;
    }
}

void test_SSTManage_with_SSTable(){// 数据很多也就同时测试了文件合并。
    SSTManage sstM("/tmp/tmpp/SSTManage");
    vector<SSTManage::SSTRecord> vc;
    for(auto& r:sstM.SSTInfoVector){
        for(auto& c:r){
            vc.push_back(c);
        }
    }
    for(auto e:vc){
        cout<<e<<endl;
    }
    cout<<"\t\t\t\t\t\t\t####################"<<endl;
    SkipList sl;
    int N=1500;
    for(int i=0;i<N;i++){
        Data data(i,1,to_string(i),to_string(i));
        sl.insert(data);
    }
    sstM.writeToSST(sl.getIterator());
}