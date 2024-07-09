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
void dyn_edge_orientation_CCHHQRS::delete_directed(NodeID source, NodeID target){}      // TODO
void dyn_edge_orientation_CCHHQRS::add(NodeID source, NodeID target){}                  // TODO
void dyn_edge_orientation_CCHHQRS::remove(NodeID source, NodeID target){}               // TODO