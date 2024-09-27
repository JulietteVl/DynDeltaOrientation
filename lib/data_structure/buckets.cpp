#include "buckets.h"

Buckets::Buckets()
{
    i_fast = 0; // Will be updated
}

Buckets::Buckets(const DeltaOrientationsConfig& config){
    this->config = config;
    i_fast = static_cast<int>(log(config.b / 4)/log(1 + config.lambda)); // d = 0
    buckets.push_back(SingleBucket(i_fast, list<BucketElement>()));
    Bi = buckets.end();
    Bi = prev(Bi);
}


Buckets::~Buckets()= default;


void Buckets::add(NodeID u, out_neighbour_iterator uv_iterator) const{
    Bi->bucket.push_back(BucketElement(u, uv_iterator));
}

void Buckets::add(NodeID u, int du, out_neighbour_iterator uv_iterator){
    int j = static_cast<int>(log(du)/log(1 + config.lambda));
    for (auto B = buckets.begin(); B != buckets.end(); ++B){ // The right bucket already exists, we found it
        if (B->bucketID == j){
            B->bucket.push_back(BucketElement(u, uv_iterator));
            return;
        }
        if (B->bucketID > j){ // we went too far
            list<BucketElement> Bj;
            Bj.push_back(BucketElement(u, uv_iterator));
            buckets.insert(B, SingleBucket(j, Bj));
            return;
        }
    }
    // Otherwise, we need a bucket with a higher bucketID:
    list<BucketElement> Bj;
    Bj.push_back(BucketElement(u, uv_iterator));
    buckets.push_back(SingleBucket(j, Bj));
}


// Remove the element that we would get_from_max_bucket()
// Should only be called if multiplicity is 0
void Buckets::remove_top(){
    buckets.rbegin()->bucket.pop_front();
}

// Should only be called if multiplicity is 0
void Buckets::remove(NodeID u, int j){
    // Find the bucket
    auto is_bucket = [&j](const SingleBucket &B) { return B.bucketID == j; };
    auto it = find_if(buckets.begin(), buckets.end(), is_bucket);
    auto Bj = it->bucket;

    // Find the vertex in the bucket
    auto is_target = [&u](const BucketElement &w) { return w.node == u; };
    auto it2 = find_if(Bj.begin(), Bj.end(), is_target);

    it->bucket.erase(it2);                // Erase the vertex from the bucket

    if (it->bucket.empty() and it->bucketID != i_fast) {  // If the bucket is empty,
        buckets.erase(it);                // Remove it
    }
}


void Buckets::update(NodeID u, int du_prev, int du, out_neighbour_iterator uv_iterator){
    // check if the outdegree of u leads to a different bucket
    int j_prev = static_cast<int>(log(du_prev)/log(1 + config.lambda));
    int j      = static_cast<int>(log(du     )/log(1 + config.lambda));
    if (j_prev == j){return;}
    // If it does, remove u in that bucket
    remove(u, j_prev);
    add(u, du, uv_iterator);
}


void Buckets::update_Bi(int dv){
    // This is the value that i_fast should have by the end of the function
    int new_i = static_cast<int> (log(max(
        (1 + config.lambda) * dv, config.b / 4
    ))/log(1 + config.lambda));
    int i_top = buckets.rbegin()->bucketID;
    if (new_i = i_fast){ return; }

    if (new_i > i_top){
        buckets.push_back(SingleBucket(new_i, list<BucketElement>()));
        Bi = prev(buckets.end());
        i_fast = new_i;
        return;
    }
    if (new_i > i_fast){
        while(i_fast < new_i & i_fast < i_top){
            Bi = next(Bi);           // Find the successor of new_i
            i_fast = Bi->bucketID;
        }
    // No bucket that has index exactly new_i? Make one
        if (i_fast > new_i){ // We went too far
            buckets.insert(Bi, SingleBucket(new_i, list<BucketElement>()));
            Bi = prev(Bi);
            i_fast = new_i;
            return;
        }
    }
    if (new_i < i_fast){
        while(new_i < i_fast){              // Find predecessor of new_i
            if (Bi == buckets.begin()){     // Cannot use prev on begin(), just solve right away.
                buckets.insert(Bi, SingleBucket(new_i, list<BucketElement>()));
                Bi = prev(Bi);              // Bi is the bucket we just inserted
                i_fast = new_i;
            }
            else{
                Bi = prev(Bi);
                i_fast = Bi->bucketID;
                // No bucket that has index exactly Bi? Make one
                if (i_fast < new_i){             // We went too far backward
                    Bi = next(Bi);          // insert will insert before pos, so we go back forward once.
                    buckets.insert(Bi, SingleBucket(new_i, list<BucketElement>()));
                    Bi = prev(Bi);          // Bi is the bucket we just inserted
                    i_fast = new_i;
                }
            }
        }
    }
}


list<pair<NodeID, int>>::iterator Buckets::get_from_max_bucket(){
    return buckets.rbegin()->bucket.begin()->out_iterator;
}
