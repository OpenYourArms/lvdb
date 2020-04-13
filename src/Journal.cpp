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