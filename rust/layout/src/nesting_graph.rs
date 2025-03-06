//! ### Idea from Sander's "Layout of Compound Directed Graphs."
//!
//! A nesting graph creates dummy nodes for the tops and bottoms of subgraphs,
//! adds appropriate edges to ensure that all cluster nodes are placed between
//! these boundaries, and ensures that the graph is connected.
//!
//! In addition, through the use of the minlen property, that nodes
//! and subgraph border nodes to not end up on the same rank.
//!
//! Pre-Conditions:
//!
//!    1. Input graph is a DAG
//!    2. Nodes in the input graph has a minlen attribute
//!
//! Post-Conditions:
//!
//!    1. The Input graph is connected.
//!    2. Dummy nodes are added for the tops and bottoms of subgraphs.
//!    3. The minlen attribute for nodes is adjusted to ensure nodes do not
//!       get placed on the same rank as subgraph border nodes.

use ahash::{HashMap, HashMapExt};

use crate::{
    Dummy::{Border, Root},
    Graph, GraphEdge, GraphNode, Key, EMPTY_ROOT,
};

impl Graph {
    pub(super) fn nesting_run(&mut self) {
        let root = self.initialize_nesting();
        let node_sep = self.calculate_node_sep();
        self.adjust_edge_minlen(node_sep);
        self.process_children(root, node_sep);
        self.config.node_rank_factor = node_sep as f32;
    }

    pub(super) fn nesting_cleanup(&mut self) {
        if let Some(root) = &self.nesting_root.clone() {
            self.remove_node(root);
        }
        self.nesting_root = None;

        for edge in self.edges() {
            if let Some(ge) = self.edge1(edge) {
                ge.nesting.then(|| self.remove_edge1(edge));
            }
        }
    }

    fn initialize_nesting(&mut self) -> Key {
        let graph_node = GraphNode::default();
        let root = self.add_dummy_node(Root, graph_node);
        self.nesting_root = Some(root);
        root
    }

    fn calculate_node_sep(&self) -> i32 {
        let (_, max_height) = self.tree_depths();
        (2 * max_height + 1) as i32
    }
    
    fn adjust_edge_minlen(&mut self, node_sep: i32) {
        for edge in self.edge_values.values_mut() {
            edge.minlen = Some(edge.minlen.unwrap() * node_sep);
        }
    }

    fn process_children(&mut self, root: Key, node_sep: i32) {
        let weight = self.sum_weights() + 1.0;
        let mut stack: Vec<Key> = self.children(&EMPTY_ROOT).into_iter().collect();

        let (depths, max_height) = self.tree_depths();

        while let Some(key) = stack.pop() {
            if self.children(&key).is_empty() {
                if key != root {
                    self.set_edge(root, key, Some(GraphEdge::with_minlen(node_sep)));
                }
                continue;
            }

            let (top, bottom) = self.link_border_nodes(key);
            self.update_node_borders(key, top, bottom);

            for k in self.children(&key) {
                let (child_top, child_bottom, this_weight) = self.nodes[&k].top_bottom(k, weight);
                let minlen =
                    if child_top == child_bottom { max_height - depths[&k] + 1 } else { 1 };
                self.add_border_edge(minlen, this_weight, top, bottom, child_top, child_bottom);
                stack.push(k);
            }

            if self.parent(&key).is_none() {
                self.set_edge(
                    root,
                    top,
                    Some(GraphEdge::with_minlen_nesting(depths[&key] + max_height)),
                );
            }
        }
    }

    fn link_border_nodes(&mut self, key: Key) -> (Key, Key) {
        let top = self.add_border_node();
        let bottom = self.add_border_node();
        self.set_parent(top, Some(key));
        self.set_parent(bottom, Some(key));
        (top, bottom)
    }

    fn update_node_borders(&mut self, key: Key, top: Key, bottom: Key) {
        if let Some(node) = self.node_mut(&key) {
            node.border_top = Some(top);
            node.border_bottom = Some(bottom);
        }
    }
}

impl GraphEdge {
    fn with_minlen(minlen: i32) -> Self {
        GraphEdge { minlen: Some(minlen), weight: Some(0.0), ..GraphEdge::default() }
    }

    fn with_minlen_nesting(minlen: usize) -> Self {
        GraphEdge {
            minlen: Some(minlen as i32),
            weight: Some(0.0),
            nesting: true,
            ..GraphEdge::default()
        }
    }
}

impl Graph {
    fn tree_depths(&self) -> (HashMap<Key, usize>, usize) {
        let mut depths: HashMap<Key, usize> = HashMap::new();
        let mut stack: Vec<(Key, usize)> = Vec::new();
        let mut max_depth: usize = 0;

        for node_id in self.children(&EMPTY_ROOT) {
            stack.push((node_id, 1));
        }

        while let Some((node_id, depth)) = stack.pop() {
            for child_id in self.children(&node_id) {
                stack.push((child_id, depth + 1));
            }
            depths.insert(node_id, depth);
            max_depth = max_depth.max(depth);
        }

        if max_depth > 0 {
            max_depth -= 1;
        }

        (depths, max_depth)
    }

    fn sum_weights(&self) -> f32 {
        let mut sum_weight: f32 = 0.0;

        for edge in self.edge_values.values() {
            if let Some(weight) = edge.weight {
                sum_weight += weight;
            }
        }

        sum_weight
    }
}

impl Graph {
    fn add_border_node(&mut self) -> Key {
        self.add_dummy_node(Border, GraphNode::default())
    }

    fn add_border_edge(
        &mut self,
        minlen: usize,
        weight: f32,
        top: Key,
        bottom: Key,
        child_top: Key,
        child_bottom: Key,
    ) {
        let top_edge = GraphEdge {
            minlen: Some(minlen as i32),
            weight: Some(weight),
            nesting: true,
            ..GraphEdge::default()
        };
        self.set_edge(top, child_top, Some(top_edge));

        let bottom_edge = GraphEdge {
            minlen: Some(minlen as i32),
            weight: Some(weight),
            nesting: true,
            ..GraphEdge::default()
        };
        self.set_edge(child_bottom, bottom, Some(bottom_edge));
    }
}

impl GraphNode {
    fn top_bottom(&self, key: Key, weight: f32) -> (Key, Key, f32) {
        let (top, this_weight) = if let Some(border_top) = self.border_top {
            (border_top, 2.0 * weight)
        } else {
            (key, weight)
        };

        let bottom = if let Some(border_bottom) = self.border_bottom { border_bottom } else { key };

        (top, bottom, this_weight)
    }
}
