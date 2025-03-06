//! Graph acyclification utilities for removing and restoring cycles.
//!
//! This module provides algorithms to convert cyclic graphs to DAGs (Directed Acyclic Graphs)
//! by finding feedback arc sets (FAS), with support for cycle restoration. Implemented algorithms:
//! - Depth-First Search (DFS) based FAS detection (`dfs_fas`)
//! - Greedy FAS algorithm (unimplemented placeholder)
//!
//! # Key Concepts
//! - **Feedback Arc Set**: Set of edges whose removal makes the graph acyclic
//! - **Edge Reversal**: Strategy to maintain graph connectivity while breaking cycles

use ahash::{HashMap, HashMapExt, HashSet, HashSetExt};
use std::cmp::Reverse;

use super::{Edge, Graph, Key};
use crate::Acyclicer::*;

impl Graph {
    /// Converts the graph to a directed acyclic graph (DAG) by reversing edges.
    ///
    /// The algorithm used depends on the configured `acyclicer`:
    /// - Greedy: Uses greedy heuristic (currently unimplemented)
    /// - Default: Uses depth-first search (DFS) to find feedback arc set
    ///
    /// # Behavior
    /// 1. Identifies feedback edges using selected algorithm
    /// 2. Removes feedback edges from original orientation
    /// 3. Reinserts edges in reversed orientation with `reversed` flag set
    ///
    /// # Notes
    /// - Original graph structure can be restored with [`restore_cycles`]
    /// - Modified edges maintain their metadata with `reversed` marker
    pub(super) fn make_acyclic(&mut self) {
        let Some(edges) = (match self.config.acyclicer {
            Greedy => Some(self.greedy_fas()),
            Dfs | NoAcyclicer => Some(self.dfs_fas()),
        }) else {
            return;
        };

        for edge in edges {
            if let Some(mut graph_edge) = self.edge1(edge).cloned() {
                self.remove_edge1(edge);
                graph_edge.reversed = true;
                self.set_edge(edge.target, edge.source, Some(graph_edge));
            }
        }
    }

    /// Finds feedback arc set using depth-first search traversal.
    ///
    /// # Algorithm
    /// 1. Maintains two visitation states: global and current traversal
    /// 2. Tracks back edges during DFS that connect to already-visited nodes
    /// 3. Collects these back edges as the feedback arc set
    ///
    /// # Complexity
    /// - Time: O(V + E)
    /// - Space: O(V)
    fn dfs_fas(&mut self) -> Vec<Edge> {
        let mut fas: Vec<Edge> = vec![];
        let mut visited: HashSet<Key> = HashSet::new();
        /// (node_id, out_edges, edge_index)
        let mut stack: Vec<(Key, Vec<Edge>, usize)> = vec![];

        for &key in self.nodes.keys() {
            if visited.contains(&key) {
                continue;
            }

            let mut local_stack: Vec<Key> = vec![key];
            let mut local_visited: HashSet<Key> = HashSet::new();
            local_visited.insert(key);

            while let Some(current_node) = local_stack.pop() {
                if visited.insert(current_node) {
                    let out_edges = self.out_edges(&current_node);
                    stack.push((current_node, out_edges, 0));
                }

                if let Some((_, out_edges, edge_index)) = stack.last_mut() {
                    if *edge_index < out_edges.len() {
                        let edge = out_edges[*edge_index];
                        *edge_index += 1;
                        match local_visited.insert(edge.target) {
                            true => local_stack.push(edge.target),
                            false => fas.push(edge),
                        }
                    } else {
                        stack.pop();
                    }
                }
            }
        }

        fas
    }

    /// Restores graph to the original cyclic state
    /// by reversing previously modified edges.
    ///
    /// # Operation
    /// 1. Scans all edges in graph
    /// 2. Flip any edge with `reversed = true` back to original orientation
    /// 3. Maintains all edge attributes during reversal
    ///
    /// # Invariants
    /// - After execution, all edges will have `reversed = false`
    /// - Graph topology returns to pre-acyclification state
    pub(super) fn restore_cycles(&mut self) -> Option<()> {
        for e in self.edges() {
            let edge = self.edge_mut1(e)?;
            if edge.reversed {
                let mut label = edge.clone();
                label.reversed = false;
                self.set_edge(e.target, e.source, Some(label));
            }
        }

        None
    }

    fn greedy_fas(&mut self) -> Vec<Edge> {
        let mut in_deg: HashMap<Key, usize> = HashMap::new();
        let mut out_deg: HashMap<Key, usize> = HashMap::new();
        let mut src_edges: HashMap<Key, Vec<Edge>> = HashMap::new();

        for edge in self.edges() {
            *out_deg.entry(edge.source).or_insert(0) += 1;
            *in_deg.entry(edge.target).or_insert(0) += 1;
            src_edges.entry(edge.source).or_default().push(edge);
        }

        let mut nodes: Vec<Key> = self.nodes.keys().copied().collect();
        nodes.sort_by_cached_key(|&n| {
            let out = out_deg.get(&n).unwrap_or(&0);
            let ins = in_deg.get(&n).unwrap_or(&0);
            ((*out as isize - *ins as isize).abs(), Reverse(n))
        });

        let mut fas = Vec::new();
        let mut removed = HashSet::new();

        while let Some(node) = nodes.pop() {
            if removed.contains(&node) || !out_deg.contains_key(&node) {
                continue;
            }

            let out = *out_deg.get(&node).unwrap_or(&0);
            let ins = *in_deg.get(&node).unwrap_or(&0);

            if out > ins {
                if let Some(edges) = src_edges.get(&node) {
                    for edge in edges {
                        if !removed.contains(&edge.target) {
                            fas.push(*edge);
                            in_deg.entry(edge.target).and_modify(|x| *x -= 1);
                        }
                    }
                }
            } else {
                for edge in self.in_edges(&node) {
                    if !removed.contains(&edge.source) {
                        fas.push(edge);
                        out_deg.entry(edge.source).and_modify(|x| *x -= 1);
                    }
                }
            }

            removed.insert(node);
            out_deg.remove(&node);
            in_deg.remove(&node);

            nodes.sort_by_cached_key(|&n| {
                let out = out_deg.get(&n).unwrap_or(&0);
                let ins = in_deg.get(&n).unwrap_or(&0);
                ((*out as isize - *ins as isize).abs(), Reverse(n))
            });
        }

        fas
    }

}
