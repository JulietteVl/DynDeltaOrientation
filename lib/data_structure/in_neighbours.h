#ifndef IN_NEIGHBOURS_H
#define IN_NEIGHBOURS_H

#include <bits/stdc++.h>

#include "DeltaOrientationsConfig.h"

struct DEdge;
struct Vertex;
struct InEdge;
struct SingleBucket;

struct InEdge {
        DEdge* e;
        unsigned int d; // estimated out degree of the source
        bool exists = true;
        InEdge(){}
        InEdge(DEdge* edge, unsigned int out_degree) { e = edge, d = out_degree; }
        bool operator == (const InEdge& a) const { return a.d == d; }
        bool operator < (const InEdge& a) const { return a.d < d; }
};

class InNeighbours {
public:
        virtual void add(DEdge* uv, int out_degree, int bucket_v) = 0;
        virtual void remove(DEdge* uv) = 0;
        virtual void update(DEdge* uv, int outdegree_u) = 0;
        virtual DEdge* get_max() = 0;
};


class Buckets : public InNeighbours {
public:
        // contains j, Bj for different j. External list in sorted order, internal
        // list Bj in arbitrary order. Bj contains in neighbours w s.t. j = log(d+(w))
        std::vector<SingleBucket> buckets;
        int max_bucketID = -1; // needed in delete
        int b;

public:
        Buckets();
        ~Buckets();
        explicit Buckets(const DeltaOrientationsConfig& config, int n);
        int get_bucket_id(int du) const;
        void add(DEdge* uv, int out_degree, int bucket_v) override;
        void remove(DEdge* uv) override;
        void update(DEdge* uv, int outdegree_u) override;
        DEdge* get_max();
};

class InHeap : public InNeighbours {
public:
        std::vector<InEdge*> in_neighbours_heap;
        std::vector<InEdge> in_neighbours_static;
        int count = 0;
        InHeap ();
        ~InHeap ();
        explicit InHeap(const DeltaOrientationsConfig& config, int n);
        void add(DEdge* uv, int out_degree, int bucket_v); // bucket_v is not used here
        void remove(DEdge* uv);
        void update(DEdge* uv, int outdegree_u);
        DEdge* get_max();
};

struct DEdge {
        DEdge* mirror; // vu
        NodeID target;
        int count = 0;
        int bucket; // location of the edge
        int location_in_neighbours;
        int location_out_neighbours; // location of edge in the out neighbour list of source
        DEdge(NodeID v) { target = v; }
        DEdge() {}
};

struct Vertex {
        std::vector<DEdge*> out_edges;
        InHeap in_edges;
        DEdge* self_loop;
        unsigned int out_degree = 0; // out degree
};

struct SingleBucket {
        int bucketID = -1;
        std::vector<DEdge*> bucket_elements;
        int prev = -1;
        int next = -1;

        SingleBucket() {}
        SingleBucket(unsigned int bucket_ID) { bucketID = bucket_ID; }
        ~SingleBucket() {}
};

#endif // BUCKETS_H
