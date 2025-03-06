use ahash::HashMap;

use super::{Context, Direction, Direction::*, Horizontal::*, Vertical::*};
use crate::{Graph, Key};

trait ExtentExt {
    fn extent(&self) -> (f32, f32);
}

impl ExtentExt for HashMap<usize, f32> {
    fn extent(&self) -> (f32, f32) {
        let iter = self.values().copied();

        let (mut min, mut max) = (f32::INFINITY, f32::NEG_INFINITY);

        for value in iter {
            min = min.min(value);
            max = max.max(value);
        }

        (min, max)
    }
}

impl Context {
    fn balance(&mut self) -> Option<()> {
        self.balanced = self.direction_map[TopLeft].clone();
        let keys: Vec<Key> = self.balanced.keys().copied().collect();

        for key in &keys {
            let mut vals: Vec<f32> =
                self.direction_map.iter().map(|dirs| dirs.get(key).copied().unwrap()).collect();
            vals.sort_by_key(|f| f.to_bits());
            let x1 = vals[1];
            let x2 = vals[2];
            let mid = self.balanced.get_mut(key)?;
            *mid = (x1 + x2) / 2.0;
        }

        None
    }

    fn min_alignment(&self, graph: &Graph) -> Option<(Direction, f32, f32)> {
        let (idx, align) = self.direction_map.iter().enumerate().min_by_key(|(_, keys)| {
            let mut max = f32::NEG_INFINITY;
            let mut min = f32::INFINITY;

            for (key, x) in keys.iter() {
                let half_width = graph.nodes[key].width / 2.0;
                max = max.max(x + half_width);
                min = min.min(x - half_width);
            }

            (max - min).to_bits()
        })?;

        let (min, max) = align.extent();

        Some((Direction::from(idx), min, max))
    }

    fn align_coordinates(&mut self, graph: &Graph) -> Option<()> {
        let (min_direction, min, max) = self.min_alignment(graph)?;

        for direction in [TopLeft, TopRight, BottomLeft, BottomRight] {
            if direction != min_direction {
                let vals = &mut self.direction_map[direction];
                let (vals_min, vals_max) = vals.extent();

                let delta = match direction.horizon() {
                    Left => min - vals_min,
                    Right => max - vals_max,
                };

                if delta != 0.0 {
                    for x in vals.values_mut() {
                        *x += delta;
                    }
                }
            }
        }

        None
    }
}

impl Graph {
    pub(super) fn position_x(&mut self, ctx: &mut Context) -> Option<()> {
        let matrix = self.key_matrix();
    
        self.find_conflict(ctx, &matrix);
        let mut matrix = matrix;

        for vertical in [Top, Bottom] {
            if vertical == Bottom {
                matrix = self.key_matrix();
                matrix.reverse()
            }

            for horizontal in [Left, Right] {
                if horizontal == Right {
                    matrix.iter_mut().for_each(|inner| inner.reverse());
                }

                self.vertical_alignment(ctx, &matrix, vertical);
                let mut compact = self.horizontal_compaction(&matrix, ctx);

                if horizontal == Right {
                    compact.values_mut().for_each(|x| *x = -*x);
                }

                ctx.direction_map[vertical + horizontal] = compact;
            }
        }

        ctx.align_coordinates(self);
        ctx.balance()
    }

    pub(super) fn position_y(&mut self) -> Option<()> {
        let matrix = self.key_matrix();
        let rank_sep = self.config.ranksep;
        let mut y = 0.0;
        for keys in matrix {
            let mut max_height = f32::NEG_INFINITY;

            for key in &keys {
                max_height = max_height.max(self.node(key)?.height)
            }

            for key in &keys {
                let node = self.node_mut(key)?;
                node.y = y + max_height / 2.0;
            }

            y += max_height + rank_sep;
        }

        None
    }
}
