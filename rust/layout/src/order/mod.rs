mod build_layer_graph;
mod cross_count;
mod init_order;
mod subgraph;

use std::ops::Range;

use build_layer_graph::EdgeRelation;

use crate::{Graph, Key, EMPTY_KEY};

impl Graph {
    pub fn order(&mut self) -> Option<()> {
        let mut matrix = self.init_order()?;
        self.assign_order(&matrix);

        let mut best_cc = f64::INFINITY;
        let mut best: Vec<Vec<Key>> = Vec::new();

        let mut last_best = 0;
        loop {
            if last_best >= 4 {
                break;
            }

            matrix = self.key_matrix();
            let cc = self.cross_count(&mut matrix) as f64;

            if cc < best_cc {
                last_best = 0;
                best = matrix;
                best_cc = cc;
            } else {
                last_best += 1;
            }
        }

        self.assign_order(&best)
    }

    #[allow(dead_code)]
    #[inline]
    fn build_layer_graphs(&mut self, ranks: Range<i32>, relationship: EdgeRelation) -> Vec<Graph> {
        ranks.map(|rank| self.build_layer_graph(rank, relationship)).collect()
    }

    fn assign_order(&mut self, matrix: &[Vec<Key>]) -> Option<()> {
        for keys in matrix {
            for (i, key) in keys.iter().enumerate() {
                self.node_mut(key)?.order = Some(i);
            }
        }

        None
    }
}

#[allow(dead_code)]
fn sweep_graphs(layer_graphs: &mut [Graph], bias_right: bool) -> Option<()> {
    let mut graph = Graph::new(true, false);

    for lg in layer_graphs {
        let root = lg.root.unwrap_or(EMPTY_KEY);
        let sorted = lg.sort_subgraph(&graph, root, bias_right)?;

        for (i, key) in sorted.keys.iter().enumerate() {
            if let Some(node) = lg.node_mut(key) {
                node.order = Some(i);
            }
        }

        lg.add_constraints(&mut graph, &sorted.keys);
    }

    None
}
