#ifndef BUCKETS_H
#define BUCKETS_H

#include <bits/stdc++.h>

#include "DeltaOrientationsConfig.h"

using namespace std;    // for my sanity. and readability.

typedef list<pair<NodeID, int>>::iterator out_neighbour_iterator; // point to v + multiplicity in G_b[u]
//typedef list<pair<NodeID, out_neighbour_iterator>> bucket;

struct BucketElement{
    // Instead of having duplicates, we refer to the multiplicity in Gb, accessible through the iterator.
    NodeID node;
    list<pair<NodeID, int>>::iterator out_iterator;
    BucketElement(){}
    BucketElement(NodeID n, out_neighbour_iterator it){node = n; out_iterator = it;}
    ~BucketElement(){}
};

struct SingleBucket{
    unsigned int bucketID;
    list<BucketElement> bucket;
    SingleBucket(){}
    SingleBucket(unsigned int bucket_ID, const list<BucketElement> &B){bucketID = bucket_ID; bucket = B;}
    ~SingleBucket(){}
};

class Buckets{
protected:
    // contains j, Bj for different j. External list in sorted order, internal list Bj in arbitrary order.
    // Bj contains in neighbours w s.t. j = log(d+(w))
    list<SingleBucket> buckets;
    // Each element of Bj, representing u as an in-neighbour of v, contains u and a pointer to the position of v and the multiplicity of uv in G_b[u]

    // Bi used for insertion / deletion during a recursion. This is an iterator.
    _List_iterator<SingleBucket> Bi;
    unsigned int i_fast;                 // index of Bi
    
public:
    DeltaOrientationsConfig config;
    Buckets();
    explicit Buckets(const DeltaOrientationsConfig& config);
    ~Buckets();
    void add(NodeID u, out_neighbour_iterator uv_iterator) const;
    void add(NodeID u, int du, out_neighbour_iterator uv_iterator);
    void remove_top();
    void remove(NodeID u, int j);
    void update(NodeID u, int du_prev, int du);  // move u in a bucket given its out degree
    void update_Bi(int dv); // Check if Bi should be updated when the outdegree of the node change.
    // Getters
    out_neighbour_iterator get_from_max_bucket();
};

#endif // BUCKETS_H