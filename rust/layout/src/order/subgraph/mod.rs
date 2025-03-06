pub mod add_constraints;
mod barycenters;
mod context;
mod resolve_conflict;
mod sort;

use ahash::{HashMap, HashMapExt};
pub use context::*;

use crate::{Graph, Key};

#[derive(Debug, Clone, Default)]
pub struct Subgraph {
    pub keys: Vec<Key>,
    pub barycenter: f32,
    pub weight: f32,
}

impl Graph {
    pub fn sort_subgraph(&self, cg: &Graph, start_key: Key, bias_right: bool) -> Option<Subgraph> {
        let mut stack: Vec<(Key, Subgraph)> = vec![(start_key, Subgraph::default())];
        let mut subgraphs: HashMap<Key, Subgraph> = HashMap::new();

        while let Some((curr_key, mut subgraph)) = stack.pop() {
            let movable = self.children(&curr_key);

            let mut barycenters = self.barycenters(&movable);
            for entry in &mut barycenters {
                if self.children(&entry.key).len() > 0 {
                    stack.push((entry.key, Subgraph::default()));
                }
            }

            let mut ctx = Context::default();
            ctx.resolve(cg, &barycenters);
            ctx.expand_subgraph(&subgraphs);

            subgraph = ctx.next_subgraph(bias_right)?;

            if stack.is_empty() {
                return Some(subgraph);
            }

            subgraphs.insert(curr_key, subgraph);
        }

        None
    }
}
