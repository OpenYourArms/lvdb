//
// Created by Administrator on 2020/4/5.
//

#ifndef LVDB_JOURNAL_H
#define LVDB_JOURNAL_H

#ifndef WIN32

#include <iostream>
#include <string>
#include <vector>

using namespace  std;

typedef unsigned long long ULL;
typedef pair<string,string> KV_p;
typedef pair<int,KV_p> OPERATION_p;

class Journal{
public:
    Journal(string filePath){}
    bool _write(OPERATION_p op);
    bool _write(vector<OPERATION_p> ops);
    bool _read();
private:
    ULL logSequence;
    ULL statementSequence;
    int fileDescriptor;

    enum {LOG_PACKAGE_HEADER_SIZE=sizeof(ULL)+sizeof(int)+sizeof(char),
            LOG_RECORD_HEADER_SIZE=sizeof(ULL)+sizeof(int)};

    struct KeyInfo{
        ULL keySeq;
        char type;
        string key;
    };
    struct operationInfo{
        char type;
        int keyLength;
        KeyInfo keyInfo;
        int valueLength;
        string value;
    };
};

void test_journal();



#endif

#endif //LVDB_JOURNAL_H
