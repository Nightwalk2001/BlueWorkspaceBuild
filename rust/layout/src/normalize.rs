//! Breaks any long edges in the graph into short segments that span 1 layer
//! each.
//! This operation is undoable with the denormalize function.
//!
//! Pre-Conditions:
//!
//!    1. The input graph is a DAG.
//!    2. Each node in the graph has a "rank" property.
//!
//! Post-Condition:
//!
//!    1. All edges in the graph have a length of 1.
//!    2. Dummy nodes are added where edges have been split into segments.
//!    3. The graph is augmented with a "dummy_chains" attribute which contains
//!       the first dummy in each chain of dummy nodes produced.

use smallvec::smallvec;

use crate::{Dummy, Dummy::EdgeProxy, Edge, Graph, GraphEdge, GraphNode, Point, EMPTY_KEY};

impl Graph {
    pub(super) fn normalize(&mut self) {
        self.dummy_chains = Some(vec![]);
        for edge in self.edges() {
            self.normalize_edge(edge);
        }
    }

    fn normalize_edge(&mut self, e: Edge) -> Option<()> {
        let mut s = e.source;
        let t = e.target;
        let mut s_rank = self.node(&s)?.rank?;
        let t_rank = self.node(&t)?.rank?;

        let edge = self.edge_mut1(e)?;
        edge.points = Some(smallvec![]);
        let weight = edge.weight;
        let rank = edge.rank.unwrap_or(0);

        self.remove_edge1(e);

        let mut i = 0;
        s_rank += 1;
        while s_rank < t_rank {
            let mut dummy_node =
                GraphNode { edge: Some(e), rank: Some(s_rank), ..GraphNode::default() };
            if s_rank == rank {
                dummy_node.dummy = Some(EdgeProxy);
            }
            let dummy_id = self.add_dummy_node(Dummy::Edge, dummy_node);
            let dummy_edge = GraphEdge { weight, ..GraphEdge::default() };
            self.set_edge(s, dummy_id, Some(dummy_edge));
            if i == 0 {
                let dummy_chains = &mut self.dummy_chains;
                dummy_chains.get_or_insert(vec![]).push(dummy_id);
            }
            s = dummy_id;
            i += 1;
            s_rank += 1;
        }

        let graph_edge = GraphEdge { weight, ..GraphEdge::default() };
        self.set_edge(s, t, Some(graph_edge));

        None
    }

    pub(super) fn denormalize(&mut self) -> Option<()> {
        if let Some(dummy_chains) = self.dummy_chains.clone() {
            for &dummy_id in &dummy_chains {
                let Some(mut node) = self.node(&dummy_id).copied() else {
                    continue;
                };

                let edge_obj = node.edge?;
                let mut prev_edge = self
                    .edge1(edge_obj)
                    .cloned()
                    .unwrap_or(GraphEdge { points: Some(smallvec![]), ..GraphEdge::default() });
                let mut curr_dummy = dummy_id;
                while node.dummy.is_some() {
                    let new_dummy =
                        self.successors(&curr_dummy).first().copied().unwrap_or(EMPTY_KEY);
                    self.remove_node(&curr_dummy);
                    prev_edge.points.as_mut()?.push(Point::of(node.x, node.y));

                    curr_dummy = new_dummy;
                    node = self.node(&curr_dummy).cloned()?;
                }

                self.set_edge1(edge_obj, Some(prev_edge));
            }
        }

        None
    }
}
