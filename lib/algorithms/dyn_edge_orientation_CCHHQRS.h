#ifndef DELTA_ORIENTATIONS_CCHHQRS
#define DELTA_ORIENTATIONS_CCHHQRS

#include <memory>
#include <map>

#include "DeltaOrientationsConfig.h"
#include "DeltaOrientationsResult.h"
#include "DynEdgeOrientation.h"
#include "dyn_graph_access.h"
#include "buckets.h"

class dyn_edge_orientation_CCHHQRS : public dyn_edge_orientation {
        public:
                dyn_edge_orientation_CCHHQRS(const std::shared_ptr<dyn_graph_access>& GOrientation, const DeltaOrientationsConfig& config,
                                DeltaOrientationsResult& result);
                void handleInsertion(NodeID source, NodeID target) override;
                void handleDeletion(NodeID source, NodeID target) override;
                void end() {
                        for( unsigned i = 0; i < GOrientation->number_of_nodes(); i++) {
                                for( unsigned j = 0; j < m_adj[i].size(); j++) {
                                        GOrientation->new_edge(i, m_adj[i][j]);
                                }
                        }
                };

                bool adjacent(NodeID source, NodeID target) {
                        for( unsigned i = 0; i < m_adj[source].size(); i++) {
                                if( m_adj[source][i] == target ) return true;
                        }
                        for( unsigned i = 0; i < m_adj[target].size(); i++) {
                                if( m_adj[target][i] == source ) return true;
                        }

                        return false;
                }
        private:
                std::vector<int> dp;                                    // out degrees
                std::vector<std::map<int, int>> G_b;                    // out neighbours and their multiplicity
                std::vector<Buckets> N_in;                              // in neighbours
                std::vector< std::vector<NodeID> > m_adj;
                void insert_directed(NodeID source, NodeID target);
                void delete_directed(NodeID source, NodeID target);
                void add(NodeID source, NodeID target);
                void remove(NodeID source, NodeID target);
               // add whatever I need
};

#endif
