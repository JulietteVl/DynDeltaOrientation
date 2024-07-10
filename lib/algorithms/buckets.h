#ifndef BUCKETS_H
#define BUCKETS_H

#include <bits/stdc++.h>

#include "DeltaOrientationsConfig.h"


class Buckets{
protected:
    std::list<std::pair<int, std::list<int>>> buckets;     // contains j, Bj
                                            // external list in sorted order, internal list Bj in arbitrary order.
                                            // Bj contains in neighbours w s.t. j = log(d+(w))
    typename std::list<std::pair<int, std::list<int>>>::iterator Bi;      // Bi used for insertion / deletion during a recursion.
    int i_top;                              // index of Bi
    
public:
    DeltaOrientationsConfig config;
    Buckets(const DeltaOrientationsConfig& config);
    ~Buckets();
    void add(int u);
    void add(int u, int du);
    void remove(int u);
    void remove(int u, int th);
    void update(int u, int du_prev, int du);  // move u in a bucket given its out degree
    void update_Bi(int dv); // Check if Bi should be updated when the outdegree of the node change.
    // Getters
    auto rbegin();
    auto rend();
    int get_from_max_bucket();
    int get_n_max_bucket();
    int get_i();
};


Buckets::Buckets(const DeltaOrientationsConfig& config){
    this->config = config;
    i_top = log(config.b / 4)/log(1 + config.lambda); // d = 0
    buckets.push_back(make_pair(i_top, std::list<int>()));
    Bi = buckets.end()--;
}


Buckets::~Buckets(){}


void Buckets::add(int u){
    Bi->second.push_back(u);
}


void Buckets::add(int u, int du){
    int j = du;
    for(auto it = buckets.begin(); it != buckets.end(); it ++){
        if (it->first = j){
            it->second.push_back(u);
        }
    }
}


void Buckets::remove(int u){
    
    Bi->second.remove(u);
}


void Buckets::remove(int u, int th){
    int j = th;
    // Find the bucket
    for(auto it = buckets.begin(); it != buckets.end(); it ++){
        if (it->first = j){
            // Find the vertex in the bucket
            for(auto it2 = it->second.begin(); it2 != it->second.end(); it ++){
                if (*it2 == u){
                    it->second.erase(it2);      // erase the vertex from the bucket
                    if (it->second.empty()){    // if the bucket is empty,
                        buckets.erase(it);      // remove it
                    }
                    return;
                }
            }
        }
    }
}


void Buckets::update(int u, int du_prev, int du){
    remove(u, du_prev);
    add(u, du);
}


void Buckets::update_Bi(int dv){
    // This is the value that i_top should have by the end of the function
    int i = log(std::max(
        (1 + config.lambda) * dv, config.b / 4
    ))/log(1 + config.lambda);

    if (i > i_top){
        while(i > i_top){
            Bi = next(Bi);      // Find the successor of i
            if (Bi == buckets.end()){
                i_top = i + 1;  // Bi.second is undefined
            }
            else{
                i_top = Bi->first;
            }
        }
        // No bucket that has index exactly i? Make one
        if (i_top > i){ // We went too far
            buckets.insert(Bi, make_pair(i, std::list<int>()));
            Bi = prev(Bi);
            i_top = i;
        }
    }
    if (i < i_top){
        while(i < i_top){               // Find predecessor of i
            if (Bi == buckets.begin()){ // Cannot use prev on begin(), just solve right away.
                buckets.insert(Bi, make_pair(i, std::list<int>()));
                Bi = prev(Bi);           // Bi is the bucket we just inserted
                i_top = i;
            }
            else{
                Bi = prev(Bi);
                i_top = Bi->first;
                // No bucket that has index exactly Bi? Make one
                if (i_top < i){             // We went too far backward
                    Bi = next(Bi);          // insert will insert before pos, so we go back forward once.
                    buckets.insert(Bi, make_pair(i, std::list<int>()));
                    Bi = prev(Bi);          // Bi is the bucket we just inserted
                    i_top = i;
                }
            }
        }

    }
}


int Buckets::get_n_max_bucket(){
    return buckets.rbegin()->first;
}


int Buckets::get_from_max_bucket(){
    return buckets.rbegin()->second.front();
}


auto Buckets::rbegin(){
    return buckets.rbegin();
}


auto Buckets::rend(){
    return buckets.rend();
}


int Buckets::get_i(){return this->i_top;}

#endif // BUCKETS_H