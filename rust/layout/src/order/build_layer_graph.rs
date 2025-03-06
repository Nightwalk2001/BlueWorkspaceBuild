//! Constructs a graph that can be used to sort a layer of nodes.
//! The graph contains all base and subgraph nodes from the request
//! layer in their original hierarchy and any edges
//! that are incident on these nodes and is of the type
//! requested by the "relationship" parameter.
//!
//! Nodes from the requested rank that don't have parents are assigned a root
//! node in the output graph, which is set in the root graph attribute.
//! This makes it easy to walk the hierarchy of movable nodes during ordering.
//!
//! Pre-conditions:
//!
//! 1. Input graph is a DAG
//! 2. Base nodes in the input graph have rank attribute
//! 3. Subgraph nodes in the input graph have minRank and maxRank attributes
//! 4. Edges have an assigned weight
//!
//! Post-conditions:
//!
//! 1. The Output graph has all nodes in the movable rank with preserved
//! hierarchy.
//! 2. Root nodes in the movable layer are made children of the node
//! indicated by the root attribute of the graph.
//! 3. Non-movable nodes incident on movable nodes, selected by the
//! relationship parameter, are included in the graph without a hierarchy.
//! 4. Edges incident on movable nodes, selected by the relationship
//! parameter, are added to the output graph.
//! 5. The weights for copied edges are aggregated as needed, since the output
//! graph isn't a multi-graph.

use crate::{unique_key, Graph, GraphEdge, GraphNode, Key};

#[allow(dead_code)]
#[derive(Debug, Copy, Clone, PartialEq)]
pub(super) enum EdgeRelation {
    In,
    Out,
}

impl Graph {
    pub(super) fn build_layer_graph(&mut self, rank: i32, relation: EdgeRelation) -> Graph {
        let root = self.create_root_node();
        let mut lg = Graph::new(true, false);
        lg.root = Some(root);

        for (&key, &node) in &self.nodes {
            if node.is_in_rank(Some(rank)) {
                lg.set_node(key, Some(node));
                lg.set_layer_parent(key, self.parent(&key), root);

                self.process_relations(&mut lg, key, relation);
            }
        }

        lg
    }

    fn set_layer_parent(&mut self, key: Key, parent: Option<Key>, root: Key) {
        match parent {
            Some(p) => self.set_parent(key, Some(p)),
            _ => self.set_parent(key, Some(root)),
        };
    }

    fn process_relations(&self, lg: &mut Graph, key: Key, relation: EdgeRelation) -> Option<()> {
        let edges = match relation {
            EdgeRelation::In => &self.in_map[&key],
            EdgeRelation::Out => &self.out_map[&key],
        };
        for &edge in edges {
            let source = if edge.source == key { edge.target } else { edge.source };
            let weight = lg.edge(source, key).and_then(|e| e.weight).unwrap_or(0.0);
            let new_weight = self.edge1(edge)?.weight.unwrap_or(0.0) + weight;
            lg.set_edge(
                source,
                key,
                Some(GraphEdge { weight: Some(new_weight), ..Default::default() }),
            );
        }

        None
    }

    fn create_root_node(&self) -> Key {
        loop {
            let key = unique_key();

            if !self.has_node(&key) {
                return key;
            }
        }
    }
}

impl GraphNode {
    #[inline]
    fn is_in_rank(&self, rank: Option<i32>) -> bool {
        self.rank == rank || self.min_rank <= rank && rank <= self.max_rank
    }
}
