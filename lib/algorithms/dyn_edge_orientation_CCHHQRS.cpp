#include "dyn_edge_orientation_CCHHQRS.h"
#include <cmath>
#include "random_functions.h"


dyn_edge_orientation_CCHHQRS::dyn_edge_orientation_CCHHQRS(const std::shared_ptr<dyn_graph_access>& GOrientation, const DeltaOrientationsConfig& config,
                                                           DeltaOrientationsResult& result)
    : dyn_edge_orientation(GOrientation, config, result) {
        m_adj.resize(GOrientation->number_of_nodes());
        dp.resize(GOrientation->number_of_nodes());
        G_b.resize(GOrientation->number_of_nodes());
        N_in.resize(GOrientation->number_of_nodes());
        for (int i = 0; i < GOrientation->number_of_nodes(); i++){
                N_in[i] = Buckets(config);
        }
}

void dyn_edge_orientation_CCHHQRS::handleInsertion(NodeID source, NodeID target){
        for (unsigned i = 0; i < config.b; i++){
                if (m_adj[source].size() < m_adj[target].size()){
                        insert_directed(source, target);
                }
                else{
                        insert_directed(target, source);
                }
        }
}

void dyn_edge_orientation_CCHHQRS::handleDeletion(NodeID source, NodeID target){
        // Find where is the target in the out neighbours of u.
        auto first = G_b[source].begin();
        auto last = G_b[source].end();
        auto is_target = [&target](const pair<NodeID, int> &v){return v.first == target;};
        // For each copy we made of the edge
        for (unsigned int i = 0; i < config.b; i++){
                // is there an edge uv?
                auto it = std::find_if(first, last, is_target);
                // delete uv if available, else vu
                if (first != last){
                        delete_directed(source, target);
                        first = next(it);       // If we find the element, we will delete it and search starting from the next element next time.
                }
                else{
                        delete_directed(target, source);
                }
        }
}

void dyn_edge_orientation_CCHHQRS::insert_directed(NodeID u, NodeID v){ // NOLINT(*-no-recursion)
        add(u, v);
        auto ux_iterator = argmin_out(u);
        NodeID x = ux_iterator->first;
        if (dp[u] > std::max(config.b / 4, (1 + config.lambda) * dp[x] + config.theta)){
                remove_fast(u, x, ux_iterator); // Quick deletion
                insert_directed(x, u);
        }
        else{
                N_in[u].update_Bi(dp[u]);
                for (auto it = G_b[u].begin(); it != G_b[u].end(); ++it){
                        NodeID w = it->first;
                        N_in[w].update(u, dp[u] - 1, dp[u]);
                }
        }
}

void dyn_edge_orientation_CCHHQRS::delete_directed(NodeID u, NodeID v){ // NOLINT(*-no-recursion)
        remove(u, v);                           // Search bucket
        auto ux_iterator = N_in[u].get_from_max_bucket();  // buckets.rbegin()->second.front()
        NodeID x = ux_iterator->first;
        if (dp[x] > std::max(config.b / 4, (1 + config.lambda) * dp[u] + config.theta)){
                add_fast(u, x, ux_iterator);                 // Quick insertion
                delete_directed(x, u);
        }
        else {
                N_in[u].update_Bi(dp[u]);
                for (auto it = G_b[u].begin(); it != G_b[u].end(); ++it){
                        NodeID w = it->first;
                        N_in[w].update(u, dp[u] + 1, dp[u]);
                }
        }
}

void dyn_edge_orientation_CCHHQRS::delete_directed_fast(NodeID u, NodeID v, list<pair<NodeID, int>>::iterator uv_iterator){
        remove_fast(u, v, uv_iterator);
        auto ux_iterator = N_in[u].get_from_max_bucket();
        NodeID x = ux_iterator->first;
        if (dp[x] > std::max(config.b / 4, (1 + config.lambda) * dp[u] + config.theta)){
                add_fast(u, x, ux_iterator);                 // Quick insertion
                delete_directed(x, u);
        }
        else {
                N_in[u].update_Bi(dp[u]);
                for (auto it = G_b[u].begin(); it != G_b[u].end(); ++it){
                        NodeID w = it->first;
                        N_in[w].update(u, dp[u] + 1, dp[u]);
                }
        }
}

void dyn_edge_orientation_CCHHQRS::add(NodeID u, NodeID v){
        dp[u]++;
        auto is_target = [&v](const pair<NodeID, int> &w){return w.first == v;};
        auto it = std::find_if(G_b[u].begin(), G_b[u].end(), is_target);
        if (it !=G_b[u].end()){
                it->second++;
        }
        else {
                G_b[u].push_back(std::make_pair(v, 1));
                auto uv_iterator = std::prev(G_b[u].end);
                N_in[v].add(u, dp[u], uv_iterator);
        }
}

void dyn_edge_orientation_CCHHQRS::add_fast(NodeID u, NodeID v, std::list<std::pair<NodeID, int>>::iterator uv_iterator){
        dp[u]++;
        if (uv_iterator->second > 0){
                uv_iterator->second++;
        }
        else {
                G_b[u].push_back(std::make_pair(v, 1));
                uv_iterator = std::prev(G_b[u].end);
                N_in[v].add(u, dp[u], uv_iterator);
        }
}

void dyn_edge_orientation_CCHHQRS::remove(NodeID u, NodeID v){
        // Find v in the out neighbours of u
        auto is_target = [&v](const pair<NodeID, int> &w){return w.first == v;};
        auto it = std::find_if(G_b[u].begin(), G_b[u].end(), is_target);
        // decrement the multiplicity
        it->second--;
        // if the new multiplicity is 0, remove the element.
        if (it->second <= 0){
                G_b[u].erase(it);
                N_in[v].remove(u, dp[u]);
        }
        dp[u]--;
}

void dyn_edge_orientation_CCHHQRS::remove_fast(NodeID u, NodeID v, std::list<std::pair<NodeID, int>>::iterator uv_iterator){
        uv_iterator->second--;
        if (uv_iterator->second <= 0){
                G_b[u].erase(uv_iterator);
                N_in[v].remove_top(); // quick deletion
        }
        dp[u]--;
}

// return the out neighbour of lowest out degree
list<pair<NodeID, int>>::iterator dyn_edge_orientation_CCHHQRS::argmin_out(NodeID source){
        auto it = G_b[source].begin();
        int mini = dp[it->first];
        auto it_mini = it;
        for (it = G_b[source].begin(); it != G_b[source].end(); ++it){
                if (dp[it->first] < mini){
                        mini = dp[it->first];
                        it_mini = it;
                }
        }
        return it_mini;
}