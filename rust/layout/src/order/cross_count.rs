//! ### Algorithm derived from Barth et al., "Bilayer Cross Counting"
//! A function that takes a layering (an array of layers, each with an array of
//! ordered nodes) and a graph and returns a weighted crossing count.
//!
//! Pre-Conditions:
//!
//! 1. Input graph must be non-multigraph, directed, and include only simple edges.
//! 2. Edges in the input graph must have assigned weights.
//!
//! Post-Conditions:
//!
//! 1. The graph and layering matrix are left unchanged.

use ahash::HashMap;

use crate::{Graph, Key};

impl Graph {
    pub fn cross_count(&mut self, matrix: &mut [Vec<Key>]) -> usize {
        let mut count = 0;

        /// Sort all the edges between the north and south layers by their position
        /// in the north layer and then the south.
        /// Map these edges to the position of their head in the south layer.
        for idx in 1..matrix.len() {
            let north_idx = idx - 1;
            let south_idx = idx;

            let south_layer = &matrix[south_idx];
            let south_pos: HashMap<Key, usize> =
                south_layer.iter().enumerate().map(|(idx, val)| (*val, idx)).collect();

            let mut south_entries: Vec<(usize, usize)> = matrix[north_idx]
                .iter()
                .flat_map(|k| {
                    self.out_map[k]
                        .iter()
                        .map(|&e| {
                            let pos = south_pos[&e.target];
                            let weight = self.edge1(e).unwrap().weight.unwrap();
                            (pos, weight as usize)
                        })
                        .collect::<Vec<_>>()
                })
                .collect();

            south_entries.sort_by_key(|e| e.0);

            let mut first_index = south_layer.len().next_power_of_two();

            let tree_size = 2 * first_index - 1;
            first_index -= 1;

            let mut tree = vec![0; tree_size];
            let mut c = 0;

            for &(f, s) in &south_entries {
                let mut idx = f + first_index;
                tree[idx] += s;

                let mut weight_sum = 0;
                while idx > 0 {
                    if idx % 2 != 0 {
                        weight_sum += tree[idx + 1];
                    }
                    idx = (idx - 1) >> 1;
                    tree[idx] += s;
                }
                c += s * weight_sum;
            }

            count += c
        }

        count
    }
}
