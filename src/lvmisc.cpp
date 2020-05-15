//
// Created by Administrator on 2020/4/17.
//
#include "lvmisc.h"
#include <vector>

void showERROR(string str,bool ex){
    cout<<str<<endl;
    cout<<errno<<" : "<<strerror(errno)<<endl;
    if(ex) exit(3);
}

template <>
void toBuffer(char buffer[],int& pos,const char* cstr){
    int len=strlen(cstr);
    memcpy(buffer+pos,cstr,len);
    pos+=len;
    buffer[pos++]='\0';
}
template <>
void toBuffer(char buffer[],int& pos,char* cstr){
    toBuffer(buffer,pos, const_cast<const char*>(cstr));
}

template <>
void toBuffer(char buffer[],int& pos,string str){
    //cout<<"call str\t:"<<str<<endl;
    toBuffer(buffer,pos,str.c_str());
}
const int ALIGN_MOD=1024;
void alignFileToMod(int fd,off_t mod){
    struct stat fileStat;
    fstat(fd,&fileStat);
    off_t fsz=fileStat.st_size;
    char buf[ALIGN_MOD];
    memset(buf,0,sizeof(buf));
    int left=fsz%mod;
    if(left){
        pwrite(fd,buf,mod-left,fsz);
    }
}

void exFileSize(int fd,off_t size){
    struct stat fileStat;
    fstat(fd,&fileStat);
    off_t fsz=fileStat.st_size;
    int left=size-fsz;
    if(left<=0) return;
    char buf[ALIGN_MOD];
    memset(buf,0,sizeof(buf));
    int mini=0;
    while(left){
        mini=min(ALIGN_MOD,left);
        pwrite(fd,buf,mini,fsz);
        fsz+=mini;
        left-=mini;
    }
}
OPERATION_p parseString(const char *str,string& op){
    // int== -1 非法，0，1，2对应Data::的0，1，2
    int opNumber=-1;
    op="";
    string key="";
    string value="";
    OPERATION_p valid={opNumber,{"valid OPERATION_p","valid OPERATION_p"}};
    int i=0;
    if(str[i]){
        // 除空格
        while(str[i]&&isblank(str[i])){
            i++;
        }
        // 操作标志 put,get,delete
        while(str[i]&&!isblank(str[i])){
            op+=str[i];
            i++;
        }
        if(op.empty()){
            return valid;
        }
        for(auto& e:op){
            e=tolower(e);
        }
        if(op=="put"){
            opNumber=Data::PUT;
        }else if(op=="get"){
            opNumber=Data::GET;
        }else if(op=="delete"){
            opNumber=Data::DELETE;
        }else{
            return valid;
        }
        // 除空格
        while(str[i]&&isblank(str[i])){
            i++;
        }
        // 取key
        if(!str[i]) return valid;
        char bound='\0';
        if(str[i]=='\''||str[i]=='\"'){
            bound=str[i];
            i++;
            while(str[i]&&str[i]!=bound){
                key+=str[i];
                i++;
            }
            // 理应为 bound
            if(str[i]!=bound) return valid;
            // 跳过bound
            i++;
        }else{
            return valid;
        }
        // 除空格
        while(str[i]&&isblank(str[i])){
            i++;
        }
        // 如果为get 或 delete, 即不等于put
        if(opNumber!=Data::PUT){
            // 理应为 '\0'
            if(str[i]) return valid;
            // 正常返回
            return {opNumber,{key,value}};
        }
        // 取value
        if(!str[i]) return valid;
        bound='\0';
        if(str[i]=='\''||str[i]=='\"'){
            bound=str[i];
            i++;
            while(str[i]&&str[i]!=bound){
                value+=str[i];
                i++;
            }
            // 理应为 bound
            if(str[i]!=bound) return valid;
            // 跳过bound
            i++;
        }else{
            return valid;
        }
        // 取空格
        while(str[i]&&isblank(str[i])){
            i++;
        }
        // 理应为 '\0'
        if(str[i]) return valid;
        // 正常返回
        return {opNumber,{key,value}};
    }else{
        return valid;
    }
}
OPERATION_p parseString(string str,string& op){
    return parseString(str.data(),op);
}
void testParseString(){
    vector<string> vc={
            "put '1' '111'",
            "put 2 222",
            "geT 3 ",
            "GET 4 4",
            "delete 5 5 55",
            "delete '6",
            "put '7' '7'",
            "Get '8'    ",
            "delete '9'   ",
    };
    string op="";
    for(auto input:vc){
        op="";
        auto rt=parseString(input,op);
        cout<<endl<<"input\t"<<input<<endl;
        cout<<"output\topNumber:\t"<<rt.first<<"\tkey:\t"<<rt.second.first<<"\tvalue:\t"<<rt.second.second<<endl;
        cout<<"op:\t"<<op<<endl<<endl;
    }
}