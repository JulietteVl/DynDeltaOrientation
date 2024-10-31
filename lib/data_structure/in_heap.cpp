#include "in_neighbours.h"
#include <vector>

InHeap::InHeap() {}

InHeap::InHeap(const DeltaOrientationsConfig& config, int n) {}

InHeap::~InHeap() {}

void InHeap::add(DEdge* uv, int out_degree, int bucket_v) {
       // We add the new edge to a static vector and to the heap
        count ++;
        in_neighbours_static.push_back(InEdge(uv, out_degree));
        InEdge* new_edge = & in_neighbours_static.back();
        in_neighbours_heap.push_back(new_edge);
        // UV can get the location of its inedge in the static array
        uv->location_in_neighbours = in_neighbours_static.size() - 1;

        // We add the element to the heap
        std::push_heap(in_neighbours_heap.begin(), in_neighbours_heap.end());
}

void InHeap::remove(DEdge* uv) {
        // We decrease the amortization count and update the in-edge in the static array
        count--;
        if (uv->location_in_neighbours == -1) {
                std::cerr << "the edge does not exists" << std::endl;
        }
        in_neighbours_static[uv->location_in_neighbours].exists = false;
        uv->location_in_neighbours = -1;

        // If our amortization count is far-removed from our actual count, we rebuild
        if(2 * count < in_neighbours_static.size() && count > 10){
                int i  = 0;
                for(int t = 0; t < in_neighbours_static.size(); ++t) {
                        if(in_neighbours_static[t].exists == true) {
                                in_neighbours_static[i] = in_neighbours_static[t];
                                in_neighbours_static[i].e->location_in_neighbours = i;
                                in_neighbours_heap[i] = &in_neighbours_static[i];
                                i++;
                        }
                }
                in_neighbours_static.resize(i);
                in_neighbours_heap.resize(i);
                std::make_heap(in_neighbours_heap.begin(), in_neighbours_heap.end());
        }
}

// update the element after incrementing or decrementing the out degree of the source
void InHeap::update(DEdge* uv, int outdegree_u) {
        remove(uv);
        add(uv, outdegree_u, 0);
}

DEdge* InHeap::get_max() {
        InEdge* max_in_edge = in_neighbours_heap.front();
        int location;

        while(max_in_edge->exists == false) {
                location = max_in_edge->e->location_in_neighbours;
                std::pop_heap(in_neighbours_heap.begin(), in_neighbours_heap.end());
                in_neighbours_heap.pop_back();
                in_neighbours_static[location] = in_neighbours_static[in_neighbours_static.size() - 1];
                in_neighbours_static.pop_back();

                max_in_edge = in_neighbours_heap.front();
        }
        return max_in_edge->e;
}