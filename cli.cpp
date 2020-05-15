//
// Created by Administrator on 2020/5/15.
//

#include "connect/link_cli.h"
#include <iostream>
#include <string>

using namespace std;

const int MY_BUFFER_SIZE=256;
void lvdb_cli();
int main(){
    lvdb_cli();
    return 0;
}
void lvdb_cli(){
    oyas::link_cli cli;
    cout<<"ffk"<<endl;
    int fd=cli.get_fd();
    char wbuf[MY_BUFFER_SIZE]={};
    char rbuf[MY_BUFFER_SIZE]={};
    string input={};
    while(true){
        getline(cin,input);
        strcpy(wbuf,input.c_str());
        write(fd,wbuf, sizeof(wbuf));
        read(fd,rbuf, sizeof(rbuf));
        cout<<rbuf<<endl;
        if(strcmp(rbuf,"SERVER:\tBye.")==0){
            close(fd);
            return;
        }
    }
}
