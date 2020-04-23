//
// Created by Administrator on 2020/4/13.
//

#include <algorithm>
#include "SkipList.h"

SkipList::SkipList():
        _maxHeight(1),
        _seed(static_cast<unsigned int>(time(nullptr))){
    srand(_seed);
    Data data(0,0,"");
    _head=newNode(data,_MAX_HEIGHT);
    for(int i=0;i<_MAX_HEIGHT;i++){
        _head->setNext(i,nullptr);
    }
}

SkipList::~SkipList(){
    Node* now=_head;
    Node* nx=nullptr;
    while(now){
        nx=now->getNext(0);
        //free(now);
        _fakeAlloc.deallocate(now,_sizeMap[now]);
        now=nx;
    }
}

bool SkipList::contains(string &key) {
    Node* node = findGreatOrEqual(key,nullptr);
    if(node){
        return equal(key,node->_data._key);
    }
    return false;
}

SkipList::Node* SkipList::findGreatOrEqual(string &k, Node** prev){
    Node* now=_head;
    int levelPos=_maxHeight-1;
    Node* nx=nullptr;
    while(true){
        nx=now->getNext(levelPos);
        if(keyIsAfterNode(k,nx)){
            now=nx;
        }else{
            if(prev) prev[levelPos]=now;
            if(levelPos==0){
                return nx;
            }
            --levelPos;
        }
    }
}
SkipList::Node* SkipList::findGreatOrEqual(Data &data, Node** prev){
    Node* now=_head;
    int levelPos=_maxHeight-1;
    Node* nx=nullptr;
    while(true){
        nx=now->getNext(levelPos);
        if(keyIsAfterNode(data,nx)){
            now=nx;
        }else{
            if(prev) prev[levelPos]=now;
            if(levelPos==0){
                return nx;
            }
            --levelPos;
        }
    }
}
SkipList::Node* SkipList::findLast() const{
    Node* now=_head;
    Node* nx=nullptr;
    int levelPos=_maxHeight-1;
    while(true){
        nx=_head->getNext(levelPos);
        if(nx==nullptr){
            if(levelPos==0) return now;
            --levelPos;
        }else{
            now=nx;
        }
    }
}

SkipList::Node* SkipList::findLessThan(string& k) const{
    Node* now=_head;
    Node* nx=nullptr;
    int levelPos=_maxHeight-1;
    while(true){
        assert(now==_head||now->_data._key<k);
        nx=now->getNext(levelPos);
        if(nx==nullptr||nx->_data._key>=k){
            if(levelPos==0) return now;
            --levelPos;
        }else{
            now=nx;
        }
    }
}
SkipList::Node* SkipList::findLessThan(Data& data) const{
    Node* now=_head;
    Node* nx=nullptr;
    int levelPos=_maxHeight-1;
    while(true){
        assert(now==_head||Data::compare(now->_data,data));
        nx=now->getNext(levelPos);
        if(nx==nullptr||(!Data::compare(nx->_data,data))){
            if(levelPos==0) return now;
            --levelPos;
        }else{
            now=nx;
        }
    }
}
void SkipList::insert(Data& data){
    Node* prev[_MAX_HEIGHT];
    Node* node=findGreatOrEqual(data._key,prev);
    node=findGreatOrEqual(data,prev);
    //assert(node==nullptr||!equal(data._key,node->_data._key));
    int height=randomHeight();
    if(height>_maxHeight){
        for(int i=_maxHeight;i<height;i++) prev[i]=_head;
        _maxHeight=height;
    }
    Node* new_node=newNode(data,height);
    for(int i=0;i<height;i++){
        new_node->setNext(i,prev[i]->getNext(i));
        prev[i]->setNext(i,new_node);
    }
}

void SkipList::showNodes(){
    Node *node=nullptr;
    cout<<"showNodes:"<<endl;
    vector<Data> vc;
    for(int i=_MAX_HEIGHT-1;i>=0;i--){
        node=_head->getNext(i);
        while(node){
            vc.push_back(node->_data);
            node=node->getNext(i);
        }
    }
    sort(vc.begin(),vc.end(),[](Data a,Data b){ return Data::compare(a,b);/*a._key<b._key;*/ });
    for(int i=0;i<vc.size();i++){
        if(i&&!equal(vc[i],vc[i-1])/*vc[i]._key!=vc[i-1]._key*/){
            cout<<endl;
        }
        cout<<vc[i]<<"\t\t";
    }
    cout<<endl;
}


void test_SkipList(){
    SkipList sl;
    sl.showNodes();
    vector<int> vc={1,3,8,6,2,4,9,7,5,19,22,14};
    for(auto e:vc){
        string k=to_string(e);
        string v=k;
        Data data(e,1,k,v);
        sl.insert(data);
    }
    Data d2(33,1,"22","22");
    sl.insert(d2);
    sl.showNodes();
}
