#include "buckets.h"
#include <vector>

Buckets::Buckets() {}

//move this outside since not different for every Bucket
static int offset;
 static int b;
static float lambda_precomp;
void setup_variables(const DeltaOrientationsConfig &config){
        // precompute lambda term
        lambda_precomp = 1.0 / (log(1 + config.lambda));
        b = config.b;
        offset = static_cast<int>(log(std::max(1.0f, static_cast<float>(4 * b))) * lambda_precomp);
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
}

// update the element after incrementing the out degree of the source
void Buckets::update(DEdge* uv, int outdegree_u) {
        int j = get_bucket_id(outdegree_u);
        int j_prev = uv->bucket;
        if (j == j_prev) { return; }
        if (j >= buckets.size()) { buckets.resize(j + 1); }
        remove(uv);
        buckets[j].bucket_elements.push_back(uv);
        uv->location_in_neighbours = (buckets[j].bucket_elements.size() - 1);
        uv->bucket = j;
}

void Buckets::remove(DEdge* uv) {
  //TODO replace with asserts
     /*   if(buckets[uv->bucket].bucket_elements.size() <= uv->location_in_neighbours || uv->location_in_neighbours == -1) {
                std::cerr << "The edge you are trying to delete is not present in the bucket at all" << std::endl;
        }
        if(buckets[uv->bucket].bucket_elements[uv->location_in_neighbours]->target != uv->target ||
                buckets[uv->bucket].bucket_elements[uv->location_in_neighbours]->mirror->target != uv->mirror->target) {
                std::cerr << "The edge you are trying to delete is not present in the bucket location" << std::endl;
        }*/
        if (buckets[uv->bucket].bucket_elements.size() > 1) {
                std::swap(buckets[uv->bucket].bucket_elements[uv->location_in_neighbours],
                          buckets[uv->bucket].bucket_elements[buckets[uv->bucket].bucket_elements.size() - 1]);
                buckets[uv->bucket].bucket_elements[uv->location_in_neighbours]
                        ->location_in_neighbours = uv->location_in_neighbours;
        }
        buckets[uv->bucket].bucket_elements.resize(buckets[uv->bucket].bucket_elements.size() - 1);
        if(buckets.size()==uv->bucket+1 && buckets[uv->bucket].bucket_elements.empty()){
                buckets.resize(buckets.size()-1);
        }
        uv->location_in_neighbours = -1;// buckets[uv->bucket].bucket_elements.size();
}

DEdge* Buckets::get_max(){
        return buckets.back().bucket_elements.front();
}
