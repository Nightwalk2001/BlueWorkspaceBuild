use std::mem;

use crate::{graph::Graph, RankDir::*};

impl Graph {
    pub fn coordinate_adjust(&mut self) {
        let rank_dir = self.config.rankdir;

        if rank_dir == LR || rank_dir == RL {
            self.swap_width_height();
        }
    }

    pub fn undo_coordinate_adjust(&mut self) {
        let rank_dir = self.config.rankdir;

        if rank_dir == BT || rank_dir == RL {
            self.reverse_y();
        }

        if rank_dir == LR || rank_dir == RL {
            self.swap_xy();
            self.swap_width_height();
        }
    }

    fn swap_width_height(&mut self) {
        for node in self.nodes.values_mut() {
            mem::swap(&mut node.width, &mut node.height);
        }
    }

    fn reverse_y(&mut self) {
        for node in self.nodes.values_mut() {
            node.y = -node.y;
        }

        for edge in self.edge_values.values_mut() {
            if let Some(points) = &mut edge.points {
                for point in points {
                    point.y = -point.y;
                }
            }
        }
    }

    fn swap_xy(&mut self) {
        for node in self.nodes.values_mut() {
            mem::swap(&mut node.x, &mut node.y);
        }

        for edge in self.edge_values.values_mut() {
            if let Some(points) = &mut edge.points {
                for point in points {
                    mem::swap(&mut point.x, &mut point.y);
                }
            }
        }
    }
}
