#include "dyn_edge_orientation_CCHHQRS.h"
#include <cmath>
#include "random_functions.h"


dyn_edge_orientation_CCHHQRS::dyn_edge_orientation_CCHHQRS(const std::shared_ptr<dyn_graph_access>& GOrientation, const DeltaOrientationsConfig& config,
                DeltaOrientationsResult& result)
        : dyn_edge_orientation(GOrientation, config, result) {
                m_adj.resize(GOrientation->number_of_nodes()); 
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

void dyn_edge_orientation_CCHHQRS::insert_directed(NodeID source, NodeID target){}      // TODO
void dyn_edge_orientation_CCHHQRS::delete_directed(NodeID u, NodeID v){
        remove(u, v);      // Search bucket to insert
        int x = argmin_out(u);
        if (dp[x] > std::max(config.b / 4, (1 + config.lambda) * dp[u] + config.theta)){
                add_fast(u, x);      // Quick insertion
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
void dyn_edge_orientation_CCHHQRS::add(NodeID source, NodeID target){}                  // TODO
void dyn_edge_orientation_CCHHQRS::add_fast(NodeID source, NodeID target){}             // TODO
void dyn_edge_orientation_CCHHQRS::remove(NodeID source, NodeID target){}               // TODO
void dyn_edge_orientation_CCHHQRS::remove_fast(NodeID source, NodeID target){}          // TODO
int  dyn_edge_orientation_CCHHQRS::argmin_out(NodeID source){
        auto it = this->G_b[source].begin();
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