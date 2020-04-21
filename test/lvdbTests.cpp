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

// SkipList
TEST(SkipList,all){
    //test_SkipList();
    SkipList sl;
    vector<int> vc={1,3,8,6,2,4,9,7,5,19,22,14};

}
// FakeAlloc
TEST(FakeAlloc,all){
    EXPECT_EQ(1,1);
}
// Journal
TEST(Journal,all){
    EXPECT_EQ(1,1);
}
