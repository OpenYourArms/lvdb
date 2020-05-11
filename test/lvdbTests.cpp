//
// Created by Administrator on 2020/4/21.
//

#include "gtest/gtest.h"
#include "../src/SkipList.h"
#include "../src/FakeAlloc.h"
#include "../src/Journal.h"
#include "../src/lvmisc.h"
#include "../src/MemTable.h"

// lvmisc.h
TEST(toBuffer,templateT){
    const int N=50;
    char buf[N]={};
    int pos=0;
    char c='t';
    toBuffer(buf,pos,c);
    ASSERT_EQ(pos,1);
    ASSERT_EQ(buf[0],c);
    int i=15;
    toBuffer(buf,pos,i);
    ASSERT_EQ(pos,1+4);
    ASSERT_EQ(*(static_cast<int*>((void*)(buf+1))),i);
    ULL ull=158;
    toBuffer(buf,pos,ull);
    ASSERT_EQ(pos,1+4+8);
    ASSERT_EQ(*(static_cast<ULL*>((void*)(buf+1+4))),ull);
}

TEST(toBuffer,cstring){
    const int N=50;
    char buf[N]={};
    int pos=0;
    int tmp=0;
    char* str="hello";
    toBuffer(buf,pos,str);
    ASSERT_EQ(pos,strlen(str)+1);
    ASSERT_STREQ(buf,str);
    char str2[10]="string";
    tmp=pos;
    toBuffer(buf,pos,str2);
    ASSERT_EQ(pos,tmp+strlen(str2)+1);
    ASSERT_STREQ(buf+tmp,str2);
    string str3="fff xxx";
    tmp=pos;
    toBuffer(buf,pos,str3.c_str());
    ASSERT_EQ(pos,tmp+strlen(str3.c_str())+1);
    ASSERT_STREQ(buf+tmp,str3.c_str());
}

TEST(exFileSize,full){
    string filePath="/home/todd/gtest.file";
    int fd=open(filePath.c_str(),O_RDWR|O_CREAT);
    int rt=chmod(filePath.c_str(),0666);
    struct stat fileStat;
    fstat(fd,&fileStat);
    const int targetSize=1778;
    ASSERT_NE(targetSize,fileStat.st_size);
    exFileSize(fd,targetSize);
    fstat(fd,&fileStat);
    ASSERT_EQ(targetSize,fileStat.st_size);
}
TEST(Data,compare){
    Data a(123,1,"afz","afz");
    Data b(456,1,"bdr","bdr");
    ASSERT_TRUE(Data::compare(a,b));
    Data c(111,1,"bdr","old");
    ASSERT_TRUE(Data::compare(b,c));
}
TEST(Data,myByteSize){
    Data a(1,1,"","");
    Data b(1,1,"1","11");
    ASSERT_EQ(a.myByteSize(),11+0);
    ASSERT_EQ(b.myByteSize(),11+3);
}
TEST(Data,getInvalidDataAndIsInvalidData){
    Data data=Data::getInvalidData();
    Data dt1(-1,-1,"InvalidKey","InvalidValue");
    EXPECT_TRUE(data.isInvalidData());
    EXPECT_TRUE(dt1.isInvalidData());
}
TEST(Data,setToBuffer){
    vector<Data> vc1;
    int n=15;
    for(int i=0;i<n;i++){
        vc1.push_back(Data());
        vc1.back()._sequenceNumber=i;
        vc1.back()._op=rand()%3;
        vc1.back()._key=to_string(i*10);
        vc1.back()._value=to_string(i*11);
    }
    int MAX=4096;
    char buf[MAX];
    int pos=0;
    for(auto e:vc1){
        e.setToBuffer(buf,pos);
    }
    ASSERT_TRUE(pos<MAX);
    int t=pos;
    pos=0;
    for(auto e:vc1){
        Data data;
        data.getFromBuffer(buf,pos);
        EXPECT_EQ(data._sequenceNumber,e._sequenceNumber);
        EXPECT_EQ(data._op,e._op);
        EXPECT_STREQ(data._key.c_str(),e._key.c_str());
        EXPECT_STREQ(data._value.c_str(),e._value.c_str());
    }
    ASSERT_EQ(t,pos);
}
// SkipList
class SkipListTest:public ::testing::Test{
protected:
    SkipList sl;
    void SetUp() override {
        vector<int> vc={1,3,8,6,2,4,9,7,5,19,22,14};
        /*
    1    : 1	1	1		1    : 1	1	1		1    : 1	1	1		1    : 1	1	1		1    : 1	1	1
    14   : 14	1	14		14   : 14	1	14
    19   : 19	1	19		19   : 19	1	19		19   : 19	1	19
    2    : 2	1	2		2    : 2	1	2
    22   : 22	1	22		22   : 22	1	22		22   : 22	1	22
    3    : 3	1	3		3    : 3	1	3		3    : 3	1	3
    4    : 4	1	4		4    : 4	1	4
    5    : 5	1	5		5    : 5	1	5		5    : 5	1	5		5    : 5	1	5		5    : 5	1	5
    6    : 6	1	6		6    : 6	1	6		6    : 6	1	6		6    : 6	1	6		6    : 6	1	6
    7    : 7	1	7		7    : 7	1	7		7    : 7	1	7
    8    : 8	1	8		8    : 8	1	8
    9    : 9	1	9		9    : 9	1	9		9    : 9	1	9		9    : 9	1	9
         *
         * */
        for(auto e:vc){
            string k=to_string(e);
            string v=k;
            Data data(e,Data::PUT,k,v);
            sl.insert(data);
        }
    }
    void TearDown() override {
        ;
    }
};
TEST_F(SkipListTest,equalData){
    Data a(11,1,"11","11");
    Data b(10,1,"11","11");
    Data c(12,1,"101","11");
    EXPECT_FALSE(sl.equal(a,b));
    EXPECT_FALSE(sl.equal(c,a));
    EXPECT_TRUE(Data::compare(a,b));
}
TEST_F(SkipListTest,keyIsAfterNodeData){
    Data data1(12,1,"22","22");
    Data data2(22,1,"22","22");
    SkipList::Node *node=sl.newNode(data1,5);
    ASSERT_FALSE(sl.keyIsAfterNode(data2,node));
}
TEST_F(SkipListTest,findGreatOrEqual){
    Data data1(12,1,"22","22");
    Data data2(22,1,"22","22");
    sl.insert(data1);
    // string
    SkipList::Node* path[SkipList::_MAX_HEIGHT];
    string s1="22";
    auto rt=sl.findGreatOrEqual(s1,path);
    EXPECT_TRUE(sl.equal(rt->_data,data2));
    auto rt1=sl.findGreatOrEqual(data1,path);
    EXPECT_TRUE(sl.equal(rt1->_data,data1));
}
TEST_F(SkipListTest,findLessThan){
    Data data1(12,1,"22","22");
    Data data2(22,1,"22","22");
    Data data3(2,1,"2","2");
    sl.insert(data1);
    string s1="22";
    auto e=sl.findLessThan(s1);
    ASSERT_TRUE(sl.equal(e->_data,data3));
    auto rt=sl.findLessThan(data1);
    ASSERT_TRUE(sl.equal(rt->_data,data2));

}
TEST_F(SkipListTest,writeIterator){
    auto it=sl.getIterator();
    Data data3(1,1,"1","1");
    Data data(14,1,"14","14");
    ASSERT_TRUE(sl.equal(it.nowPos->_data,data3));
    ASSERT_TRUE(sl.equal((it++).nowPos->_data,data3));
    ASSERT_TRUE(sl.equal(it.nowPos->_data,data));
}
TEST_F(SkipListTest,all){
    //test_SkipList();
    SkipList::Node* path[SkipList::_MAX_HEIGHT];
    string key1=to_string(7);
    auto e=sl.findGreatOrEqual(key1,path);
    ASSERT_STREQ(e->_data._key.c_str(),key1.c_str());
    string key2="11";
    auto e2=sl.findGreatOrEqual(key2,path);
    ASSERT_STREQ(e2->_data._key.c_str(),"14");
    string key3="21",key4="2";
    auto e3=sl.findGreatOrEqual(key4,path);
    Data data(atoi(key3.c_str()),Data::PUT,key3,key3);
    sl.insert(data);
    ASSERT_TRUE(sl.keyIsAfterNode(key3,e3));
}
// FakeAlloc
class FakeAllocTest:public ::testing::Test{
protected:
    FakeAlloc fa;
    void SetUp() override {
        ;
    }
    void TearDown() override {
        ;
    }
};
TEST_F(FakeAllocTest,roundUp){
    std::size_t size1=12;
    size_t size2=fa.roundUp(size1);
    ASSERT_EQ(size2,32);
    size1=47;
    size2=fa.roundUp(size1);
    ASSERT_EQ(size2,32*2);
}
TEST_F(FakeAllocTest,getIndex){
    //ASSERT_EQ(fa.getIndex(0),0);
    ASSERT_EQ(fa.getIndex(1),0);
    ASSERT_EQ(fa.getIndex(32),0);
    ASSERT_EQ(fa.getIndex(33),1);
    ASSERT_EQ(fa.getIndex(63),1);
    ASSERT_EQ(fa.getIndex(64),1);
    ASSERT_EQ(fa.getIndex(fa._MAX_BLOCK),fa._INDEX_SIZE-1);
}
// Journal
TEST(Journal,all){
    EXPECT_EQ(1,1);
}
// MemTable
TEST(MemTable,findTable){
    MemTable mt;
    ASSERT_EQ(mt.findTable(),0);
    mt.myTable[0].setNextStatus();
    ASSERT_EQ(mt.findTable(),1);
    mt.myTable[1].setNextStatus();
    ASSERT_EQ(mt.findTable(),-1);
}
TEST(MemTable,saveData){// writeSSTable 函数暂时为空，跳表大小为100
    MemTable mt;
    int n=0;
    for(int i=0;i<100;i++){
        Data data(i,1,to_string(i),to_string(i));
        if(mt.saveData(data)<0){
            n=i+1;
            break;
        }
    }
    // 0-6   -> (11+2)*7=91 over
    // 7-9   -> (11+2)*3=39 new
    // 10-13 -> (11+4)*4=60 over
    // 14 return=-1,i=14,n=15;
    ASSERT_EQ(n,15);
}