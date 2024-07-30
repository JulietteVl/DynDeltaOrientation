#include "buckets.h"

Buckets::Buckets(const DeltaOrientationsConfig& config){
    this->config = config;
    i_top = log(config.b / 4)/log(1 + config.lambda); // d = 0
    buckets.push_back(make_pair(i_top, std::list<int>()));
    Bi = buckets.end()--;
}


Buckets::~Buckets(){}


void Buckets::add(int u){
    Bi->second.push_back(u);
}


void Buckets::add(int u, int du){
    int j = du;
    for(auto it = buckets.begin(); it != buckets.end(); it ++){
        if (it->first = j){
            it->second.push_back(u);
        }
    }
}


// Remove the element that we would get_from_max_bucket()
void Buckets::remove_top(){
    buckets.rbegin()->second.pop_front();
}


void Buckets::remove(int u, int th){
    int j = th;
    // Find the bucket
    for(auto it = buckets.begin(); it != buckets.end(); it ++){
        if (it->first = j){
            // Find the vertex in the bucket
            for(auto it2 = it->second.begin(); it2 != it->second.end(); it ++){
                if (*it2 == u){
                    it->second.erase(it2);      // erase the vertex from the bucket
                    if (it->second.empty()){    // if the bucket is empty,
                        buckets.erase(it);      // remove it
                    }
                    return;
                }
            }
        }
    }
}


void Buckets::update(int u, int du_prev, int du){
    remove(u, du_prev);
    add(u, du);
}


void Buckets::update_Bi(int dv){
    // This is the value that i_top should have by the end of the function
    int i = log(std::max(
        (1 + config.lambda) * dv, config.b / 4
    ))/log(1 + config.lambda);

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
            buckets.insert(Bi, make_pair(i, std::list<int>()));
            Bi = prev(Bi);
            i_top = i;
        }
    }
    if (i < i_top){
        while(i < i_top){               // Find predecessor of i
            if (Bi == buckets.begin()){ // Cannot use prev on begin(), just solve right away.
                buckets.insert(Bi, make_pair(i, std::list<int>()));
                Bi = prev(Bi);           // Bi is the bucket we just inserted
                i_top = i;
            }
            else{
                Bi = prev(Bi);
                i_top = Bi->first;
                // No bucket that has index exactly Bi? Make one
                if (i_top < i){             // We went too far backward
                    Bi = next(Bi);          // insert will insert before pos, so we go back forward once.
                    buckets.insert(Bi, make_pair(i, std::list<int>()));
                    Bi = prev(Bi);          // Bi is the bucket we just inserted
                    i_top = i;
                }
            }
        }

    }
}


int Buckets::get_from_max_bucket(){
    return buckets.rbegin()->second.front();
}


auto Buckets::rbegin(){
    return buckets.rbegin();
}


auto Buckets::rend(){
    return buckets.rend();
}