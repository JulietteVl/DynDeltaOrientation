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
    Buckets();
    Buckets(const DeltaOrientationsConfig& config);
    ~Buckets();
    void add(int u);
    void add(int u, int du);
    void remove_top();
    void remove(int u, int th);
    void update(int u, int du_prev, int du);  // move u in a bucket given its out degree
    void update_Bi(int dv); // Check if Bi should be updated when the outdegree of the node change.
    // Getters
    auto rbegin();
    auto rend();
    int get_from_max_bucket();
};

#endif // BUCKETS_H