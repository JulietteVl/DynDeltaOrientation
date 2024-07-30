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
        // TODO initialise buckets with config
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
        for (unsigned i = 0; i < config.b; i++){
                if (G_b[source].count(target) > 0){
                        delete_directed(source, target);
                }
                else{
                        delete_directed(target, source);
                }
        }
}

void dyn_edge_orientation_CCHHQRS::insert_directed(NodeID u, NodeID v){
        add(u, v);
        int x = argmin_out(u);
        if (dp[u] > std::max(config.b / 4, (1 + config.lambda) * dp[x] + config.theta)){
                remove_fast(u, x); // Quick deletion
                insert_directed(x, u);
        }
        else{
                N_in[u].update_Bi(dp[u]);
                for (auto it = G_b[u].begin(); it != G_b[u].end(); it++){
                        int w = it->first;
                        N_in[w].update(u, dp[u] - 1, dp[u]);
                }
        }
}

void dyn_edge_orientation_CCHHQRS::delete_directed(NodeID u, NodeID v){
        remove(u, v);                           // Search bucket
        int x = N_in[u].get_from_max_bucket();  // buckets.rbegin()->second.front()
        if (dp[x] > std::max(config.b / 4, (1 + config.lambda) * dp[u] + config.theta)){
                add_fast(u, x);                 // Quick insertion
                delete_directed(x, u);
        }
        else {
                N_in[u].update_Bi(dp[u]);
                for (auto it = G_b[u].begin(); it != G_b[u].end(); it++){
                        int w = it->first;
                        N_in[w].update(u, dp[u] + 1, dp[u]);
                }
        }
}

void dyn_edge_orientation_CCHHQRS::delete_directed_fast(NodeID u, NodeID v){
        remove_fast(u, v);
        int x = N_in[u].get_from_max_bucket();  // buckets.rbegin()->second.front()
        if (dp[x] > std::max(config.b / 4, (1 + config.lambda) * dp[u] + config.theta)){
                add_fast(u, x);                 // Quick insertion
                delete_directed(x, u);
        }
        else {
                N_in[u].update_Bi(dp[u]);
                for (auto it = G_b[u].begin(); it != G_b[u].end(); it++){
                        int w = it->first;
                        N_in[w].update(u, dp[u] + 1, dp[u]);
                }
        }
}

void dyn_edge_orientation_CCHHQRS::add(NodeID u, NodeID v){
        dp[u]++;
        if (G_b[u].count(v) == 0){
                G_b[u][v] = 1;
                N_in[v].add(u, dp[u]);
        }
        else {
                G_b[u][v]++;
        }
}

void dyn_edge_orientation_CCHHQRS::add_fast(NodeID u, NodeID v){
        dp[u]++;
        if (G_b[u].count(v) == 0){
                G_b[u][v] = 1;
                N_in[v].add(u);
        }
        else {
                G_b[u][v]++;
        }
}

void dyn_edge_orientation_CCHHQRS::remove(NodeID u, NodeID v){
        G_b[u][v]--;
        if (G_b[u].count(v) == 0){
                auto it = G_b[u].find(v);
                G_b[u].erase(it);
                N_in[v].remove(u, dp[u]);
        }
        dp[u]--;
}

void dyn_edge_orientation_CCHHQRS::remove_fast(NodeID u, NodeID v){
        G_b[u][v]--;
        if (G_b[u].count(v) == 0){
                auto it = G_b[u].find(v);
                G_b[u].erase(it);
                N_in[v].remove_top(); // quick deletion
        }
        dp[u]--;
}

int dyn_edge_orientation_CCHHQRS::argmin_out(NodeID source){
        auto it = G_b[source].begin();
        int mini = it->second;
        int x = it->first;
        for (it; it != G_b[source].end(); it++){                
                if (it->second < mini){
                        mini = it->second;
                        x = it->first;
                }
        }
        return x;
}