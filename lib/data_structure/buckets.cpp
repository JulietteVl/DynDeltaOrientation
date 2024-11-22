#include "buckets.h"
#include <vector>

Buckets::Buckets() {}
double lambda_precomp;

Buckets::Buckets(const DeltaOrientationsConfig& config, int n) {
        // precompute lambda term
        lambda_precomp = config.lambda_precomp;
        b = config.b;
        offset = config.offset;
}

Buckets::~Buckets() = default;

int Buckets::get_bucket_id(const int du) const {
        float duf = du;
        if (duf < 4 * b) { return 0; }
        return static_cast<int>(log(duf) * lambda_precomp) - offset;
}

void Buckets::add(DEdge* uv, int out_degree, int bucket_v) {
        int j = get_bucket_id(out_degree);
        if (j >= buckets.size()) {
                buckets.resize(j + 1);
        }

        buckets[j].bucket_elements.push_back(uv);
        uv->location_in_neighbours = buckets[j].bucket_elements.size() - 1;
        uv->bucket = j;

        // The bucket already exists (prev and next are up to date)
        if (buckets[j].bucketID == j) { return; }

        // The bucket was unused - prev and next have to be set
        buckets[j].bucketID = j;

        // Check if j is the new max bucket
        if (max_bucketID == -1) { max_bucketID = j; }
        else if (j > max_bucketID) {
                buckets[max_bucketID].next = j;
                buckets[j].prev = max_bucketID;
                max_bucketID = j;
        }
        // Or insert
        else {
                int j_self = bucket_v;
                int pos = max_bucketID;
                int pos2 = -1;
                if (j <= j_self) {
                        pos = j_self;
                        pos2 = buckets[j_self].next;
                } // the position will be very close to j in the cases where the insertion
                // has to be fast
                while (pos > j) {
                        pos2 = pos;
                        pos = buckets[pos].prev;
                }
                if (pos > -1) { // j is not the lowest bucket
                        buckets[pos].next = j;
                }
                buckets[j].prev = pos;
                buckets[j].next = pos2;
                buckets[pos2].prev = j;
        }
}

// update the element after incrementing the out degree of the source
void Buckets::update(DEdge* uv, int outdegree_u) {
        int j = get_bucket_id(outdegree_u);
        int j_prev = uv->bucket;
        if (j == j_prev) { return; }
        if (j >= buckets.size()) { buckets.resize(j + 1); }
        if (j > max_bucketID) { max_bucketID = j; }

        if (buckets[j].bucketID == -1) { // bucket j does not exist yet. We will first insert it and then
                // update the edges, possibly removing j_prev.
                if (j < j_prev) {
                        int successor = j_prev;
                        while(j < buckets[successor].prev) {
                                successor = buckets[successor].prev;
                        }
                        int p = buckets[successor].prev;
                        if (p != -1) { // j_prev was not the lowest bucket
                                buckets[p].next = j;
                        }
                        buckets[j].prev = p;
                        buckets[successor].prev = j;
                        buckets[j].next = successor;
                }
                else {
                        int predecessor = j_prev;
                        while(j  > buckets[predecessor].next && buckets[predecessor].next != -1 ) {
                                predecessor = buckets[predecessor].next;
                        }
                        int n = buckets[predecessor].next;
                        if (n != -1) { // j_prev was not the highest bucket
                                buckets[n].prev = j;
                        }
                        buckets[j].next = n;
                        buckets[predecessor].next = j;
                        buckets[j].prev = predecessor;
                }
        }

        remove(uv);

        buckets[j].bucketID = j;
        buckets[j].bucket_elements.push_back(uv);
        uv->location_in_neighbours = (buckets[j].bucket_elements.size() - 1);
        uv->bucket = j;
}

void Buckets::remove(DEdge* uv) {
        if(buckets[uv->bucket].bucket_elements.size() <= uv->location_in_neighbours || uv->location_in_neighbours == -1) {
                std::cerr << "The edge you are trying to delete is not present in the bucket at all" << std::endl;
        }
        if(buckets[uv->bucket].bucket_elements[uv->location_in_neighbours]->target != uv->target ||
                buckets[uv->bucket].bucket_elements[uv->location_in_neighbours]->mirror->target != uv->mirror->target) {
                std::cerr << "The edge you are trying to delete is not present in the bucket location" << std::endl;
        }
        if (buckets[uv->bucket].bucket_elements.size() > 1) {
                std::swap(buckets[uv->bucket].bucket_elements[uv->location_in_neighbours],
                          buckets[uv->bucket].bucket_elements[buckets[uv->bucket].bucket_elements.size() - 1]);
                buckets[uv->bucket].bucket_elements[uv->location_in_neighbours]
                        ->location_in_neighbours = uv->location_in_neighbours;
        }
        buckets[uv->bucket].bucket_elements.resize(buckets[uv->bucket].bucket_elements.size() - 1);

        uv->location_in_neighbours = -1;// buckets[uv->bucket].bucket_elements.size();

        int j = uv->bucket;

        if (buckets[j].bucket_elements.empty()) {
                int p = buckets[j].prev;
                int n = buckets[j].next;
                if (n != -1) { // j is not max bucket
                        buckets[n].prev = buckets[j].prev;
                }
                if (p != -1) { // j is not lowest bucket
                        buckets[p].next = buckets[j].next;
                }
                if (j == max_bucketID) { // because of the self loop, there is always a bucket
                        max_bucketID = buckets[max_bucketID].prev;
                        buckets.resize(max_bucketID + 1);
                }

                buckets[j].bucketID = -1;
                buckets[j].next = -1;
                buckets[j].prev = -1;
        }
}

DEdge* Buckets::get_max(){
        return buckets[max_bucketID].bucket_elements.front();
}
