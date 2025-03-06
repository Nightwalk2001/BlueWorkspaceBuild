//! ### Gansner et al., "A Technique for Drawing Directed Graphs"
//! Assigns an initial order value for each node by performing a DFS search
//! starting from nodes in the first rank.
//! Nodes are assigned an order in their rank as they're first visited.
//!
//! Returns a layering matrix with an array per layer and each layer sorted by
//! the order of its nodes.

use ahash::{HashSet, HashSetExt};

use crate::{Graph, Key};

impl Graph {
    pub(super) fn init_order(&self) -> Option<Vec<Vec<Key>>> {
        let mut visited: HashSet<Key> = HashSet::new();
        let mut simple_nodes: Vec<Key> =
            self.nodes.keys().filter(|k| self.children(k).is_empty()).copied().collect();

        let mut max_rank = 0;
        for id in &simple_nodes {
            if let Some(rank) = self.nodes[id].rank {
                max_rank = max_rank.max(rank)
            }
        }

        let mut layers: Vec<Vec<Key>> = vec![Vec::new(); max_rank as usize + 1];

        simple_nodes.sort_by_key(|id| Some(self.nodes[id].rank?));

        for id in simple_nodes {
            let mut stack = vec![id];

            while let Some(id) = stack.pop() {
                if !visited.insert(id) {
                    continue;
                }

                let rank = self.nodes[&id].rank? as usize;
                layers[rank].push(id);

                stack.extend(self.successors(&id));
            }
        }

        Some(layers)
    }
}
