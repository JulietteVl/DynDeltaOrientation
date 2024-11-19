#include "dyn_edge_orientation_CCHHQRS.h"
#include "random_functions.h"
#include <cmath>

dyn_edge_orientation_CCHHQRS::dyn_edge_orientation_CCHHQRS(
        const std::shared_ptr<dyn_graph_access>& GOrientation,
        const DeltaOrientationsConfig& config, DeltaOrientationsResult& result)
        : dyn_edge_orientation(GOrientation, config, result) {
        if (config.lambda == 0 || config.theta == 0 && config.b < 1 / config.lambda) {
                std::cerr << "Illegal choice of parameters" << std::endl;
        }
        robin_size = std::max(10.0, 2.0 / config.lambda) + 1;

        m_adj.resize(GOrientation->number_of_nodes());
        vertices.resize(GOrientation->number_of_nodes());
        for (int i = 0; i < GOrientation->number_of_nodes(); i++) {
                vertices[i].in_edges = Buckets(config, GOrientation->number_of_nodes());
                vertices[i].self_loop = new DEdge(i);
                edge_allocator.push_back(vertices[i].self_loop);
                vertices[i].self_loop->mirror = vertices[i].self_loop;
                add(vertices[i].self_loop, i);
        }
}

void dyn_edge_orientation_CCHHQRS::handleInsertion(NodeID source, NodeID target) {
        if (source == target) { return; }

        // make the edges
        DEdge* new_uv = new DEdge(target);
        DEdge* new_vu = new DEdge(source);
        edge_allocator.push_back(new_uv);
        edge_allocator.push_back(new_vu);

        new_uv->mirror = new_vu;
        new_vu->mirror = new_uv;

        for (unsigned i = 0; i < config.b; i++) {
                if (vertices[source].out_degree <= vertices[target].out_degree) { insert_directed_worst_case(new_uv, source); }
                else { insert_directed_worst_case(new_vu, target); }
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
}

// Naive version
void dyn_edge_orientation_CCHHQRS::insert_directed(DEdge* uv, NodeID u) { // NOLINT(*-no-recursion)
        add(uv, u);
        DEdge* uw_min = vertices[u].out_edges.back();

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
        add(uv, u);
        int t_max = 0;
        int old_robin = vertices[u].robin;
        // Round robin
        for (int t = 0; t < std::min(robin_size, vertices[u].out_degree); t++) {
                if (vertices[u].robin >= vertices[u].out_edges.size()){ vertices[u].robin = 0; }
                DEdge* uw = vertices[u].out_edges[vertices[u].robin];
                // Find
                if (vertices[u].out_degree > std::max(
                        static_cast<float>(config.b),
                        (1 + config.lambda) * vertices[uw->target].out_degree + config.theta
                        )){
                        remove(uw, u);
                        insert_directed_worst_case(uw->mirror, uw->target);
                        vertices[u].robin++;
                        if (vertices[u].robin >= vertices[u].out_edges.size()){ vertices[u].robin = 0; }
                        t_max = t;
                        break;
                }
                vertices[u].robin++;
                if (vertices[u].robin >= vertices[u].out_edges.size()){ vertices[u].robin = 0; }
        }

        // Update all edges visited in the previous loop
        for (int t = 0; t <= std::max(t_max, 5); t++) {
                if (old_robin >= vertices[u].out_edges.size()) { old_robin = 0; }
                DEdge* uw = vertices[u].out_edges[old_robin];
                vertices[uw->target].in_edges.update(uw, vertices[u].out_degree);
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
                        if (vertices[u].robin >= vertices[u].out_edges.size()) { vertices[u].robin = 0; }
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
                vertices[u].out_edges.push_back(uv);
                uv->location_out_neighbours = vertices[u].out_edges.size() - 1;
                // insert u in the in neighbours of v
                vertices[uv->target].in_edges.add(uv, vertices[u].out_degree, vertices[uv->target].self_loop->bucket);
        }
}

void dyn_edge_orientation_CCHHQRS::remove(DEdge* uv, NodeID u) {
        // decrement the multiplicity of the edge
        uv->count--;

        int robin = vertices[u].robin;
        int current_location = uv->location_out_neighbours;
        // if the new multiplicity is 0, remove the element.
        if (uv->count == 0) {
                if (vertices[u].out_edges.size() > 1) {
                        if (current_location >= robin - 1) {
                                std::swap(vertices[u].out_edges[current_location],
                                    vertices[u].out_edges[vertices[u].out_edges.size() - 1]);
                                vertices[u].out_edges[current_location]->location_out_neighbours = current_location;
                                if (current_location == robin - 1) { robin --; }
                        }
                        else {
                                // swap robin element with the element to delete
                                std::swap(vertices[u].out_edges[current_location],
                                    vertices[u].out_edges[robin]);
                                vertices[u].out_edges[current_location]->location_out_neighbours = current_location;
                                // swap the element to delete (at robin location) with the end
                                std::swap(vertices[u].out_edges[vertices[u].out_edges.size() - 1],
                                    vertices[u].out_edges[robin]);
                                vertices[u].out_edges[robin]->location_out_neighbours = robin;
                                robin --;
                        }
                }
                vertices[u].out_edges.resize(vertices[u].out_edges.size() - 1);
                uv->location_out_neighbours = -1; //vertices[u].out_edges.size();
                vertices[uv->target].in_edges.remove(uv);
        }
        if (uv->count < 0) { std::cerr << "Error: edge count below 0" << std::endl; }
        vertices[u].out_degree--;
        vertices[u].robin = robin;
}
