//
// Created by Administrator on 2020/4/24.
//

#include "SSTManage.h"
#include "SSTable.h"

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
    int rsz=0;
    for(auto& r:SSTInfoVector){
        for(auto& c:r){
            int l=0;
            rsz=c.myBufferSize();
            toBuffer(_buffer,l,rsz);
            c.setToBuffer(_buffer,l);
            pwrite(_fileDescriptor,_buffer,l,pos);
            pos+=l;
        }
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
    _levelSizeVector[0]+=record.fileSize;
    SSTInfoVector[0].push_back(record);
    _counter++;
    _fileSize+=record.myBufferSize();
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

void test_SSTManage_with_SSTable(){
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