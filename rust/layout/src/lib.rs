#![feature(let_chains)]
#![allow(unused_doc_comments)]

mod acyclic;
mod algo;
mod coordinate_system;
pub mod graph;
mod nesting_graph;
mod normalize;
mod order;
mod parent_dummy_chains;
mod position;
mod rank;
mod selfedge;
mod utils;

pub use graph::*;
use mimalloc::MiMalloc;
use utils::*;

/// ### Performance
/// When there are many nodes, most of the performance consumption
/// is Vec, HashMap memory allocation and memory transfer when expanding.
///
/// And it's often memory allocation of very large objects.
///
/// [`mimalloc`] is also challenging to achieve large performance improvement,
/// but at the initial stage of expansion, it can Improve performance
#[global_allocator]
static GLOBAL: MiMalloc = MiMalloc;

pub fn layout(graph: &mut Graph) {
    graph.make_space_for_edge_labels();
    graph.remove_self_edges();
    graph.make_acyclic();
    graph.nesting_run();
    let mut ncg: Graph = graph.as_non_compound();
    ncg.rank();
    ncg.transfer_node_edges(graph);
    graph.remove_empty_ranks();
    graph.nesting_cleanup();
    graph.normalize_ranks();
    graph.assign_rank_min_max();
    graph.remove_edge_proxies();
    graph.normalize();
    graph.parent_dummy_chains();
    graph.order();
    graph.insert_self_edges();
    graph.coordinate_adjust();
    graph.position();
    graph.position_self_edges();
    graph.denormalize();
    graph.undo_coordinate_adjust();
    graph.translate_graph();
    graph.assign_node_intersects();
    graph.reverse_points_for_reversed_edges();
    graph.restore_cycles();
}
