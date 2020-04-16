//
// Created by Administrator on 2020/4/13.
//

#ifndef LVDB_SKIPLIST_H
#define LVDB_SKIPLIST_H

#ifndef WIN32

#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>
#include <ctime>
#include <vector>
using namespace std;

typedef unsigned long long ULL;

struct Data{
    string _key;
    string _value;
    ULL _sequenceNumber;
    int _op;
    Data()= default;
    Data(int seq,int op,string k,string v=""):_sequenceNumber(seq),_op(op),_key(k),_value(v){}
    friend ostream& operator<<(ostream& os,Data& a){
        return os<<setiosflags(ios::left)<<setw(4)<<a._key<<" : "<<a._value;
    }
};

class SkipList{

    enum {_MAX_HEIGHT=5};
struct Node{
    Data _data;
    Node* _next[1];

    Node(Data data){ _data=data; }
    inline Node* getNext(int n){
        assert(n>=0);
        return _next[n];
    }
    inline void setNext(int n, Node* node){
        assert(n>=0);
        _next[n]=node;
    }
};
public:
    SkipList();
    ~SkipList();
    inline int randomHeight(){ return rand()%_MAX_HEIGHT + 1;}
    bool contains(string& key);
    inline bool equal(string& a,string& b){ return a==b; }
    Node* findGreatOrEqual(string &k, Node** prev);
    Node* findLast() const;
    Node* findLessThan(string& k) const;
    inline int getMaxHeight() const{ return _maxHeight; }
    void insert(Data& data);
    inline bool keyIsAfterNode(string& k,Node* n)const{ return n&&k>n->_data._key; }
    Node* newNode(Data data,int height){
        int sz= sizeof(Node) + sizeof(Node*)*(height-1);
        char* p=(char*)malloc(sz);
        return (Node*)new(p) Node(data);
    }
    void showNodes();
private:
    int _maxHeight;
    unsigned int _seed;// init
    Node* _head;
};

void test_SkipList();



#endif

#endif //LVDB_SKIPLIST_H
