//! Initializes ranks for the input graph using the longest path algorithm.
//!
//! This algorithm scales well and is fast in practice, it yields rather poor
//! solutions.
//!
//! Nodes are pushed to the lowest layer possible, leaving the bottom
//! ranks wide and leaving edges longer than necessary.
//!
//! However, due to its speed, this algorithm is good for getting an initial
//! ranking that can be fed into other algorithms.
//!
//! This algorithm doesn't normalize layers because it'll be used by other
//! algorithms in most cases.
//! If using this algorithm directly, be sure to run normalize at the end.
//!
//! Pre-conditions:
//!
//!    1. Input graph is a DAG.
//!    2. Input graph node labels can be assigned properties.
//!
//! Post-conditions:
//!
//!    1. Each node has an (unnormalized) "rank" property.

use ahash::{HashSet, HashSetExt};

use self::Variant::*;
use crate::{Graph, Key};

enum Variant {
    Array(Vec<Key>),
    Single(Key),
}

impl Graph {
    pub(super) fn longest_path(&mut self) {
        let mut visited = HashSet::new();
        let init = self.in_map.iter().filter(|(_, vec)| vec.is_empty()).map(|(&k, _)| k).collect();
        let mut stack = vec![Array(init)];

        while stack.len() > 0 {
            let curr = stack.last_mut().unwrap();

            match curr {
                Array(arr) => {
                    let k = arr.pop().unwrap();
                    if arr.is_empty() {
                        stack.pop();
                    }

                    if !visited.contains(&k) {
                        visited.insert(k);
                        let children: Vec<Key> =
                            self.out_edges(&k).iter().map(|e| e.target).rev().collect();
                        if children.len() > 0 {
                            stack.push(Single(k));
                            stack.push(Array(children))
                        } else {
                            self.node_mut(&k).unwrap().rank = Some(0);
                        }
                    }
                }
                Single(k) => {
                    let k = k.clone();
                    stack.pop();
                    let mut rank = i32::MAX;

                    for &edge in &self.out_map[&k] {
                        let minlen = self.edge1(edge).unwrap().minlen.unwrap();
                        let target_rank = self.nodes[&edge.target].rank.unwrap();

                        rank = rank.min(target_rank - minlen);
                    }

                    self.node_mut(&k).unwrap().rank = Some(rank);
                }
            }
        }
    }
}
