#ifndef BUCKETS_H
#define BUCKETS_H

#include <bits/stdc++.h>

#include "DeltaOrientationsConfig.h"


class Buckets{
protected:
    // contains j, Bj for different j. External list in sorted order, internal list Bj in arbitrary order.
    // Bj contains in neighbours w s.t. j = log(d+(w))
    std::list<std::pair<int, std::list<std::pair<int, int*>>>> buckets;
    // Each element of Bj, representing u as an in neighbour of v, contains u and a pointer to the multiplicity of v in G_b[u]

    // Bi used for insertion / deletion during a recursion. This is an iterator.
    typename std::list<std::pair<int, std::list<std::pair<int, int*>>>>::iterator Bi;
    int i_top;                 // index of Bi
    
public:
    DeltaOrientationsConfig config;
    Buckets();
    explicit Buckets(const DeltaOrientationsConfig& config);
    ~Buckets();
    void add(int u, int*v_ptr) const;
    void add(int u, int du, int*v_ptr);
    void remove_top();
    int* remove(int u, int th);
    void update(int u, int du_prev, int du);  // move u in a bucket given its out degree
    void update_Bi(int dv); // Check if Bi should be updated when the outdegree of the node change.
    // Getters
    auto rbegin();
    auto rend();
    int get_from_max_bucket();
};

#endif // BUCKETS_H