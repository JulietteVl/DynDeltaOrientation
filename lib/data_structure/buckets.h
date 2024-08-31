#ifndef BUCKETS_H
#define BUCKETS_H

#include <bits/stdc++.h>

#include "DeltaOrientationsConfig.h"

using namespace std;    // for my sanity. and readability.

class Buckets{
protected:
    // contains j, Bj for different j. External list in sorted order, internal list Bj in arbitrary order.
    // Bj contains in neighbours w s.t. j = log(d+(w))
    list<pair<int, list<pair<NodeID, list<pair<NodeID, int>>::iterator>>>> buckets;
    // Each element of Bj, representing u as an in-neighbour of v, contains u and a pointer to the position of v and the multiplicity of uv in G_b[u]

    // Bi used for insertion / deletion during a recursion. This is an iterator.
    list<pair<int, list<pair<NodeID, list<pair<NodeID, int>>::iterator>>>>::iterator Bi;
    int i_top;                 // index of Bi
    
public:
    DeltaOrientationsConfig config;
    Buckets();
    explicit Buckets(const DeltaOrientationsConfig& config);
    ~Buckets();
    void add(NodeID u, list<pair<NodeID, int>>::iterator uv_iterator) const;
    void add(NodeID u, int du, list<pair<NodeID, int>>::iterator uv_iterator);
    void remove_top();
    list<pair<NodeID, int>>::iterator remove(NodeID u, int j);
    void update(NodeID u, int du_prev, int du);  // move u in a bucket given its out degree
    void update_Bi(int dv); // Check if Bi should be updated when the outdegree of the node change.
    // Getters
    _List_iterator<pair<NodeID, int>> get_from_max_bucket();
};

#endif // BUCKETS_H