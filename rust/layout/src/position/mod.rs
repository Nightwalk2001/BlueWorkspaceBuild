mod align;
mod block_graph;
mod conflict;
mod context;
mod xy;

use block_graph::*;
use context::*;

use crate::Graph;

impl Graph {
    pub(super) fn position(&mut self) -> Option<()> {
        let mut ncg: Graph = self.as_non_compound();

        let mut ctx = Context::default();
        ncg.position_y();
        ncg.position_x(&mut ctx);

        for (key, &x) in &ctx.balanced {
            let node = self.node_mut(key)?;
            node.x = x;
            node.y = ncg.nodes[key].y;
        }

        None
    }
}
