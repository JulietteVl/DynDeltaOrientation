#include "dyn_edge_orientation_CCHHQRS.h"
#include "random_functions.h"
#include <cmath>

dyn_edge_orientation_CCHHQRS::dyn_edge_orientation_CCHHQRS(
        const std::shared_ptr<dyn_graph_access>& GOrientation,
        const DeltaOrientationsConfig& config, DeltaOrientationsResult& result)
        : dyn_edge_orientation(GOrientation, config, result) {
        if (config.lambda == 0 || config.theta == 0 && config.b < 1 / config.lambda) {
                std::cerr << "Illegal choice of parameters" << std::endl;
                std::cout<<config.b<<" "<<config.lambda<<" "<< 1 / config.lambda<<std::endl;

        }
        robin_size = std::max(10.0, 2.0 / config.lambda) + 1;

        m_adj.resize(GOrientation->number_of_nodes());
        vertices.resize(GOrientation->number_of_nodes());
        setup_variables(config);
        edge_arena.resize(2*(GOrientation->number_of_ops())+ GOrientation->number_of_nodes());

        for (int i = 0; i < GOrientation->number_of_nodes(); i++) {
                vertices[i].in_edges = Buckets();
                vertices[i].self_loop = &edge_arena[edge_arena_ptr++];
                vertices[i].self_loop->target=i;
                vertices[i].self_loop->mirror = vertices[i].self_loop;
                add(vertices[i].self_loop, i);
        }
}

void dyn_edge_orientation_CCHHQRS::handleInsertion(NodeID source, NodeID target) {
        if (source == target) { return; }

        // make the edges
        assert(edge_arena_ptr<edge_arena.size());
        DEdge* new_uv = &edge_arena[edge_arena_ptr++];
         new_uv->target = target ;
        DEdge* new_vu =&edge_arena[edge_arena_ptr++];
        new_vu->target = source;

        new_uv->mirror = new_vu;
        new_vu->mirror = new_uv;

        for (unsigned i = 0; i < config.b; i++) {
                if (vertices[source].out_degree <= vertices[target].out_degree) { insert_directed_worst_case(new_uv, source); }
                else { insert_directed_worst_case(new_vu, target); }
                //Insert Directed is currently not adjusted for this new structure
        }
}

void dyn_edge_orientation_CCHHQRS::handleDeletion(NodeID source, NodeID target) {
        if (source == target) { return; }
        DEdge* uv = NULL;
        DEdge* vu = NULL;

        for (DEdge* d : vertices[source].out_edges) {
                if (d->target == target) {
                        uv = d;
                        break;
                }
        }
        if (uv != NULL) { vu = uv->mirror; }
        else {
                for (DEdge* d : vertices[target].out_edges) {
                        if (d->target == source) {
                                vu = d;
                                break;
                        }
                }
                uv = vu->mirror;
        }
        while (uv->count + vu->count > 0) {
                if (uv->count >= vu->count) { delete_directed_worst_case(uv, source); }
                else { delete_directed_worst_case(vu, target); }
        }
        //remove out of arrays
        auto & v = vertices[vu->mirror->target];
        auto & u = vertices[uv->mirror->target];

if(uv->location_out_neighbours!=-1){

        std::swap(u.out_edges[u.out_edges.size()-1],u.out_edges[uv->location_out_neighbours]);
        u.out_edges[uv->location_out_neighbours]->location_out_neighbours = uv->location_out_neighbours;
        u.out_edges.pop_back();
}
if(vu->location_out_neighbours!=-1){
assert(v.out_edges.size()>0);
assert(vu->location_out_neighbours<v.out_edges.size());
assert(vu->location_out_neighbours>=0);

        std::swap(v.out_edges[v.out_edges.size()-1],v.out_edges[vu->location_out_neighbours]);
        v.out_edges[vu->location_out_neighbours]->location_out_neighbours = vu->location_out_neighbours;
        v.out_edges.pop_back();
        }
}

// Naive version
void dyn_edge_orientation_CCHHQRS::insert_directed(DEdge* uv, NodeID u) { // NOLINT(*-no-recursion)
        add(uv, u);
         DEdge* uw_min = vertices[u].out_edges.front();
        // DEdge* uw_min = vertices[u].out_edges.back();

        // find the out neighbour of minimum out degree
        for (DEdge* uw : vertices[u].out_edges) {
                // check threshod + ignore the self loop
                if (vertices[uw->target].out_degree < vertices[uw_min->target].out_degree &&
                        uw->target != u) { uw_min = uw; }
        }

        if (vertices[u].out_degree > std::max(static_cast<float>(config.b),
                                              (1 + config.lambda) * vertices[uw_min->target].out_degree + config.
                                              theta)) {
                remove(uw_min, u);
                insert_directed(uw_min->mirror, uw_min->target);
        }
        else {
                for (DEdge* uw : vertices[u].out_edges) {
                        vertices[uw->target].in_edges.update(uw, vertices[u].out_degree);
                }
        }
}

// Naive version
void dyn_edge_orientation_CCHHQRS::delete_directed(
        DEdge* uv, NodeID u) { // NOLINT(*-no-recursion)
        remove(uv, u);
        DEdge* ux = vertices[u].in_edges.get_max();
        if (vertices[ux->target].out_degree > std::max(static_cast<float>(config.b),
                                                       (1 + config.lambda) * vertices[u].out_degree + config.theta)) {
                add(ux, u);
                delete_directed(ux->mirror, ux->target);
        }
        else {
                for (DEdge* uw : vertices[u].out_edges) {
                        vertices[uw->target].in_edges.update(uw, vertices[u].out_degree);
                }
        }
}

// Worst case version
void dyn_edge_orientation_CCHHQRS::insert_directed_worst_case(DEdge* uv, NodeID u) {
        // NOLINT(*-no-recursion)
     //   assert_edges(u);
        add(uv, u);
      //  assert_edges(u);
        int t_max = 0;
        int old_robin = vertices[u].robin;
        // Round robin
       // unsigned int out_edges_size = vertices[u].out_edges.size();
        auto max_val_loop= std::min(robin_size, vertices[u].active_edges);
        const auto out_deg = vertices[u].out_degree;
        auto& u_= vertices[u];
        for (int t = 0; t < max_val_loop; t++) {
                if (u_.robin >= vertices[u].active_edges){ u_.robin = 0; }
                assert(u_.robin<u_.active_edges);
                DEdge* uw = u_.out_edges[u_.robin];
                // Find
                if (u_.out_degree > std::max(
                       (double) config.b,
                        vertices[uw->target].out_degree *(1.0 + config.lambda) + config.theta
                        )){
                        remove(uw, u);
                        insert_directed_worst_case(uw->mirror, uw->target);
                        u_.robin++;
                        if (u_.robin >=vertices[u].active_edges){ u_.robin = 0; }
                        t_max = t;
                        break;
                }
                u_.robin++;//= ( u_robin+1) % u_.out_edges.size();
        }
         if (u_.robin >=vertices[u].active_edges){ u_.robin = 0; }
       // u_.robin = u_robin;
        // Update all edges visited in the previous loop
       //if(t_max>0) std::cout<<t_max<<std::endl;
        for (int t = 0; t <= t_max; t++) {
                if (old_robin >= vertices[u].active_edges) { old_robin = 0; }
                DEdge* uw = u_.out_edges[old_robin];
                vertices[uw->target].in_edges.update(uw, u_.out_degree);
                old_robin ++;
        }
}

// Worst case version
void dyn_edge_orientation_CCHHQRS::delete_directed_worst_case(
        DEdge* uv, NodeID u) { // NOLINT(*-no-recursion)
        remove(uv, u);
        DEdge* ux = vertices[u].in_edges.get_max();
        if (vertices[ux->target].out_degree > std::max(static_cast<float>(config.b),
                                                       (1 + config.lambda) * vertices[u].out_degree + config.theta)) {
                add(ux, u);
                delete_directed_worst_case(ux->mirror, ux->target);
                                                       }
        else {
                for (int t = 0; t < robin_size; t++) {
                        if (vertices[u].robin >= vertices[u].active_edges) { vertices[u].robin = 0; }
                        DEdge* uw = vertices[u].out_edges[vertices[u].robin];
                        vertices[uw->target].in_edges.update(uw, vertices[u].out_degree);
                        vertices[u].robin ++;
                }
        }
}

void dyn_edge_orientation_CCHHQRS::add(DEdge* uv, NodeID u) {
        vertices[u].out_degree++;
        uv->count++;
        if (uv->count == 1) { // new edge
                if(uv->location_out_neighbours==-1){
                      // std::cout<<"inserting "<<(size_t)uv<<std::endl;
                        uv->location_out_neighbours = vertices[u].out_edges.size();
                        vertices[u].out_edges.push_back(uv);
                }
                if(vertices[u].active_edges < vertices[u].out_edges.size()-1){
                        //std::cout<<"swapping "<<(size_t)uv<<std::endl;
                        assert(vertices[u].active_edges<=uv->location_out_neighbours);
                        assert(vertices[u].out_edges[uv->location_out_neighbours]==uv);
                        std::swap(vertices[u].out_edges[vertices[u].active_edges],vertices[u].out_edges[uv->location_out_neighbours]);
                        vertices[u].out_edges[uv->location_out_neighbours]->location_out_neighbours = uv->location_out_neighbours;
                        uv->location_out_neighbours = vertices[u].active_edges;           
                }
                
                vertices[u].active_edges++;
                assert(vertices[u].out_edges[vertices[u].active_edges-1]==uv);
                // insert u in the in neighbours of v
                vertices[uv->target].in_edges.add(uv, vertices[u].out_degree, vertices[uv->target].self_loop->bucket);
        } 
}

void dyn_edge_orientation_CCHHQRS::remove(DEdge* uv, NodeID u) {
        // decrement the multiplicity of the edge
        assert(uv->count>0);
        uv->count--;
 if (vertices[u].robin >=vertices[u].active_edges){ vertices[u].robin = 0; }
        int robin = vertices[u].robin;
        int current_location = uv->location_out_neighbours;

        assert(current_location<vertices[u].active_edges);
        // if the new multiplicity is 0, remove the element.
        if (uv->count == 0) {
              //  del_events++;
                if (vertices[u].out_edges.size() > 1) {
                        if (current_location >= robin - 1) {
                                std::swap(vertices[u].out_edges[current_location],
                                    vertices[u].out_edges[vertices[u].active_edges-1]);
                                vertices[u].out_edges[current_location]->location_out_neighbours = current_location;
                                if (current_location == robin - 1) { robin --; }
                        }
                        else {
                                // swap robin element with the element to delete
                                std::swap(vertices[u].out_edges[current_location],
                                    vertices[u].out_edges[robin]);
                                vertices[u].out_edges[current_location]->location_out_neighbours = current_location;
                                // swap the element to delete (at robin location) with the end
                                std::swap(vertices[u].out_edges[vertices[u].active_edges-1],
                                    vertices[u].out_edges[robin]);
                                vertices[u].out_edges[robin]->location_out_neighbours = robin;
                                robin --;
                        }
                }
                //vertices[u].out_edges.resize(vertices[u].out_edges.size() - 1);
                vertices[u].active_edges--;
                uv->location_out_neighbours = vertices[u].active_edges; //vertices[u].out_edges.size();
                vertices[uv->target].in_edges.remove(uv);
        }
        if (uv->count < 0) { std::cerr << "Error: edge count below 0" << std::endl; }
        vertices[u].out_degree--;
        vertices[u].robin = robin;
}
