#ifndef BUCKETS_H
#define BUCKETS_H

#include <bits/stdc++.h>

#include "DeltaOrientationsConfig.h"

// using namespace std; // for my sanity. and readability.

struct DEdge;
struct Vertex;
struct SingleBucket;

class Buckets {
public:
        // contains j, Bj for different j. External list in sorted order, internal
        // list Bj in arbitrary order. Bj contains in neighbours w s.t. j = log(d+(w))
        std::vector<SingleBucket> buckets;

public:
        Buckets();
        ~Buckets();
        int get_bucket_id(int du) const;
        void add(DEdge* uv, int out_degree, int bucket_v);
        void remove(DEdge* uv);
        void update(DEdge* uv, int outdegree_u);
        DEdge* get_max();
};
void setup_variables(const DeltaOrientationsConfig& config);
struct DEdge {
        DEdge* mirror; // vu
        // Vertex *source;
        NodeID target;
        int count = 0;
        int bucket; // location of the edge
        int location_in_neighbours;
        int location_out_neighbours=-1; // location of edge in the out neighbour list of
        // source
        DEdge(NodeID v) { target = v; };
        DEdge() {}
};

struct Vertex {
        std::vector<DEdge*> out_edges;
      //  std::set<DEdge*> deleted_out_edges;
        Buckets in_edges;
        DEdge* self_loop;
        unsigned int out_degree = 0; // out degree, //TODO remove
        unsigned int active_edges=0;
        unsigned int robin = 0;
};

struct SingleBucket {
        std::vector<DEdge*> bucket_elements;
        SingleBucket() {}
        ~SingleBucket() {}
};

#endif // BUCKETS_H
