#include "buckets.h"

Buckets::Buckets()
{
    i_top = 0; // Will be updated
}

Buckets::Buckets(const DeltaOrientationsConfig& config){
    this->config = config;
    i_top = static_cast<int>(log(config.b / 4)/log(1 + config.lambda)); // d = 0
    buckets.push_back(make_pair(i_top, std::list<std::pair<int, std::list<std::pair<NodeID, int>>::iterator>>()));
    Bi = buckets.end();
    Bi = prev(Bi);
}


Buckets::~Buckets()= default;


void Buckets::add(NodeID u, std::list<std::pair<NodeID, int>>::iterator uv_iterator) const{
    Bi->second.push_back(std::make_pair(u, uv_iterator));
}
std::list<std::pair<int, std::list<
        std::pair<int, int*>    // Each element of Bj, representing u as an in neighbour of v,
    >>> buckets;

void Buckets::add(NodeID u, int du, std::list<std::pair<NodeID, int>>::iterator uv_iterator){
    int j = du;
    for(auto & bucket : buckets){
        if (bucket.first == j){
            bucket.second.push_back(std::make_pair(u, uv_iterator));
        }
    }
}


// Remove the element that we would get_from_max_bucket()
void Buckets::remove_top(){
    buckets.rbegin()->second.pop_front();
}


std::list<std::pair<NodeID, int>>::iterator Buckets::remove(NodeID u, int j){
    // Find the bucket
    auto is_bucket = [&j](const pair<int, list<pair<int,list<pair<NodeID, int>>::iterator>>> &bucket){return bucket.first == j;};
    auto it = &find_if(buckets.begin, buckets.end(), is_bucket);
    auto Bj = it->second;
    // Find the vertex in the bucket
    auto is_target = [&u](const pair<int,list<pair<NodeID, int>>::iterator> &w){return w.first == u;};
    auto it2 = std::find_if(Bj.begin(), Bj.end(), is_target);
    auto uv_iterator = it2->second;
    it->second.erase(it2);          // erase the vertex from the bucket
    if (it->second.empty()){        // if the bucket is empty,
        buckets.erase(it);          // remove it
    }
    return uv_iterator;
}


void Buckets::update(NodeID u, int du_prev, int du){
    auto uv_iterator = remove(u, du_prev);
    add(u, du, uv_iterator);
}


void Buckets::update_Bi(int dv){
    // This is the value that i_top should have by the end of the function
    int i = static_cast<int> (log(std::max(
        (1 + config.lambda) * dv, config.b / 4
    ))/log(1 + config.lambda));

    if (i > i_top){
        while(i > i_top){
            Bi = next(Bi);      // Find the successor of i
            if (Bi == buckets.end()){
                i_top = i + 1;  // Bi.second is undefined
            }
            else{
                i_top = Bi->first;
            }
        }
        // No bucket that has index exactly i? Make one
        if (i_top > i){ // We went too far
            buckets.insert(Bi, make_pair(i, std::list<std::pair<int, std::list<std::pair<NodeID, int>>::iterator>>()));
            Bi = prev(Bi);
            i_top = i;
        }
    }
    if (i < i_top){
        while(i < i_top){               // Find predecessor of i
            if (Bi == buckets.begin()){ // Cannot use prev on begin(), just solve right away.
                buckets.insert(Bi, make_pair(i, std::list<std::pair<int, std::list<std::pair<NodeID, int>>::iterator>>()));
                Bi = prev(Bi);           // Bi is the bucket we just inserted
                i_top = i;
            }
            else{
                Bi = prev(Bi);
                i_top = Bi->first;
                // No bucket that has index exactly Bi? Make one
                if (i_top < i){             // We went too far backward
                    Bi = next(Bi);          // insert will insert before pos, so we go back forward once.
                    buckets.insert(Bi, make_pair(i, std::list<std::pair<int, std::list<std::pair<NodeID, int>>::iterator>>()));
                    Bi = prev(Bi);          // Bi is the bucket we just inserted
                    i_top = i;
                }
            }
        }

    }
}


 list<pair<NodeID, int>>::iterator Buckets::get_from_max_bucket(){
    return &buckets.rbegin()->second.front().first();
}