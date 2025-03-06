use ahash::{HashMap, HashMapExt, HashSet, HashSetExt};

use super::{BlockGraph, Context, Vertical, Vertical::*};
use crate::{Graph, Key, KeyCodecExt};

impl Graph {
    pub(super) fn vertical_alignment(
        &self,
        ctx: &mut Context,
        matrix: &[Vec<Key>],
        vertical: Vertical,
    ) {
        let mut pos: HashMap<Key, usize> = HashMap::new();

        for keys in matrix {
            for (order, &key) in keys.iter().enumerate() {
                ctx.root.insert(key, key);
                ctx.align.insert(key, key);
                pos.insert(key, order);
            }
        }

        for keys in matrix {
            let mut prev_idx: i32 = -1;
            for &key in keys {
                let mut neighbors: Vec<Key> = match vertical {
                    Top => self.predecessors(&key),
                    Bottom => self.successors(&key),
                };

                if neighbors.is_empty() {
                    continue;
                }

                /// Here we can improve performance a little by **unwrap**
                neighbors.sort_by_key(|id| pos.get(id));
                let mid = (neighbors.len() as f32 - 1.0) / 2.0000001;
                let start = mid.floor() as usize;
                let end = mid.ceil() as usize;
                for idx in start..=end {
                    let neighbor = neighbors[idx];
                    if ctx.align[&key] == key
                        && prev_idx < (pos[&neighbor] as i32)
                        && !ctx.has_conflict(key, neighbor)
                    {
                        let x = ctx.root[&neighbor];
                        ctx.align.insert(neighbor, key);
                        ctx.align.insert(key, x);
                        ctx.root.insert(key, x);

                        prev_idx = pos[&neighbor] as i32;
                    }
                }
            }
        }
    }

    pub(super) fn horizontal_compaction(
        &self,
        matrix: &[Vec<Key>],
        ctx: &Context,
    ) -> HashMap<Key, f32> {
        let mut compact: HashMap<Key, f32> = HashMap::new();
        let block: BlockGraph = self.build_block_graph(matrix, &ctx.root);

        let mut stack = block.nodes();
        let mut visited: HashSet<Key> = HashSet::new();
        while let Some(k) = stack.pop() {
            if visited.contains(&k) {
                let in_edges = &block.in_edges[&k];
                let mut val: f32 = 0.0;
                for key in in_edges {
                    let source = key.source();
                    let ev: f32 = compact[&source] + block.edges[key];
                    val = val.max(ev)
                }

                compact.insert(k, val);
            } else {
                visited.insert(k);
                stack.push(k);
                stack.extend(block.predecessors(&k));
            }
        }

        for &k in ctx.align.values() {
            compact.insert(k, compact[&ctx.root[&k]]);
        }

        compact
    }
}
