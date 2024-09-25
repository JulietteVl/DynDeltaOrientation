#ifndef DELTA_ORIENTATIONS_CCHHQRS
#define DELTA_ORIENTATIONS_CCHHQRS

#include <memory>
#include <bits/stdc++.h>

#include "DeltaOrientationsConfig.h"
#include "DeltaOrientationsResult.h"
#include "DynEdgeOrientation.h"
#include "dyn_graph_access.h"
#include "buckets.h"

class dyn_edge_orientation_CCHHQRS : public dyn_edge_orientation {
        public:
                dyn_edge_orientation_CCHHQRS(const shared_ptr<dyn_graph_access>& GOrientation, const DeltaOrientationsConfig& config,
                                DeltaOrientationsResult& result);
                void handleInsertion(NodeID source, NodeID target) override;
                void handleDeletion(NodeID source, NodeID target) override;
                void end() override {
                        for( unsigned i = 0; i < GOrientation->number_of_nodes(); i++) {
                                for (auto p:G_b[i]){
                                        // "complete" edges
                                        for (int j = 0; j < p.second / config.b; j++){
                                                m_adj[i].push_back(p.first);
                                        }
                                        // "partial" edges
                                        // if there is a half edge, we break ties arbitrarily
                                        if (2 * (p.second % config.b) == config.b){
                                                if (i > p.first){ m_adj[i].push_back(p.first); }
                                        }
                                        else if (p.second % config.b > config.b / 2){ m_adj[i].push_back(p.first); }
                                }
                        }
                        for( unsigned i = 0; i < GOrientation->number_of_nodes(); i++) {
                                for( unsigned j = 0; j < m_adj[i].size(); j++) {
                                        GOrientation->new_edge(i, m_adj[i][j]);
                                }
                        }
                };

                bool adjacent(NodeID source, NodeID target) override {
                        for(unsigned int i : m_adj[source]) {
                                if( i == target ) return true;
                        }
                        for(unsigned int i : m_adj[target]) {
                                if( i == source ) return true;
                        }
                        return false;
                }
        private:
                vector< vector<NodeID> > m_adj;
                vector<int> dp;                                 // out degrees
                vector<list<pair<NodeID, int>>> G_b;            // out neighbours + multiplicity
                vector<Buckets> N_in;                           // in neighbours
                void insert_directed(NodeID u, NodeID v);
                void delete_directed(NodeID u, NodeID v);
                void delete_directed_fast(NodeID u, NodeID v, list<pair<NodeID, int>>::iterator uv_iterator);
                void add(NodeID u, NodeID v);
                void add_fast(NodeID u, NodeID v, list<pair<NodeID, int>>::iterator uv_iterator);
                void remove(NodeID u, NodeID v);
                void remove_fast(NodeID u, NodeID v, list<pair<NodeID, int>>::iterator uv_iterator);
                list<pair<NodeID, int>>::iterator argmin_out(NodeID source);
};

#endif
