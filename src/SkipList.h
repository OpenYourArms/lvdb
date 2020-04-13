//
// Created by Administrator on 2020/4/13.
//

#ifndef LVDB_SKIPLIST_H
#define LVDB_SKIPLIST_H

#ifndef WIN32

#include <iostream>
#include <string>
#include <cassert>
using namespace std;

typedef unsigned long long ULL;

struct Data{
    string _key;
    string _value;
    ULL _sequenceNumber;
    int _op;
    Data(int seq,int op,string k,string v=""):_sequenceNumber(seq),_op(op),_key(key),_value(v){}
};

class SkipList{

    enum {_MAX_HEIGHT=12};
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
    SkipList(){};
    ~SkipList(){};
    inline int randomHeight(){
        return rand()%_MAX_HEIGHT + 1;
    }
    bool contains(string& key) const;
    inline bool equal(string& a,string& b){ return a==b; }
    Node* findGreatOrEqual(string &k, Node** prev) const;
    Node* findLast() const;
    Node* findLessThan(string& k) const;
    inline int getMaxHeight() const{ return _maxHeight; }
    void insert(string& k);
    inline bool keyIsAfterNode(string& k,Node* n)const{ return k>n->_data._key; }
    Node* newNode(Data data,int height);
private:
    int _maxHeight;
    unsigned int _seed;// init
};


#endif

#endif //LVDB_SKIPLIST_H
