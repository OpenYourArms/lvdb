//
// Created by Administrator on 2020/4/21.
//

#include "gtest/gtest.h"
#include "../src/SkipList.h"
#include "../src/FakeAlloc.h"
#include "../src/Journal.h"
#include "../src/lvmisc.h"

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
