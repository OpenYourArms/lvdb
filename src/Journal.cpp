//
// Created by Administrator on 2020/4/5.
//

#include "Journal.h"

bool Journal::_write(OPERATION_p op) {
    int opType=op.first;
    string key=op.second.first;
    string value=op.second.second;
    cout<<opType<<"\t"<<key<<"\t"<<value<<endl;
}

void test_journal(){
    string fpth="some path";
    OPERATION_p opp={1,{"kk","vv"}};
    Journal journal(fpth);
    journal._write(opp);
}