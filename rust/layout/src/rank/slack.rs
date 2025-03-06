use crate::{Edge, Graph};

/// The slack is defined as the
/// difference between the length of the edge and its minlen.
impl Graph {
    pub(super) fn slack(&self, edge: Edge) -> i32 {
        let source_rank = self.nodes[&edge.source].rank.unwrap();
        let target_rank = self.nodes[&edge.target].rank.unwrap();
        let minlen = self.edge_values[&edge.to_key()].minlen.unwrap();
        target_rank - source_rank - minlen
    }
}
