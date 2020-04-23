//
// Created by Administrator on 2020/3/26.
//

#ifndef ALGODEMO_MISC_H
#define ALGODEMO_MISC_H

#ifndef WIN32

#include <string>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
using namespace std;
namespace oyas{
    string get_time_str()
    {
        time_t rawtime;
        struct tm timeinfo;
        char buffer [128] = {0};
        time (&rawtime);
        localtime_r(&rawtime,&timeinfo);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return string(buffer);
    }
    void wt_log(string path,string text){
        auto fp=open(path.c_str(),O_CREAT|O_RDWR|O_APPEND,0600);
        string tmp=get_time_str();
        tmp+="\t\t";
        tmp+=text;
        tmp+="\n";
        if(fp>0){
            write(fp,tmp.c_str(), sizeof(tmp.c_str()));
        }
        if(fp>0){
            close(fp);
        }
    }


}
/*
    cout<<"argc:\t"<<endl;
    for(int i=1;i<argc;i++){
        cout<<argv[i]<<endl;
    }
    int fd=-1;
    if(argc>1){
        cout<<"cli is running!"<<endl;
        oyas::link_cli lc;
        fd=lc.get_fd();
        char buf[128];
        bool f=true;
        while(f){
            memset(buf,0, sizeof(buf));
            cin>>buf;
            write(fd,buf, sizeof(buf));
            cout<<"\t\t\t\tWrite OK"<<endl;
            read(fd,buf, sizeof(buf));
            cout<<buf<<endl;
            cout<<"\t\t\t\tMSG OK!"<<endl;
        }
    }else{
        cout<<"server is running!"<<endl;
        oyas::link_server ls;
        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_size=sizeof(clnt_addr);
        fd=ls.i_accept(&clnt_addr,clnt_addr_size);
        char buf[128];
        bool flag=true;
        while(flag){
            memset(buf,0, sizeof(buf));
            read(fd,buf, sizeof(buf));
            cout<<"GET \t"<<buf<<endl;
            buf[0]='I';
            buf[1]='C';
            buf[2]='U';
            write(fd,buf, sizeof(buf));
            cout<<"\t\t\t\t\tdone!"<<endl<<endl;
        }
    }*/

#endif

#endif //ALGODEMO_MISC_H
