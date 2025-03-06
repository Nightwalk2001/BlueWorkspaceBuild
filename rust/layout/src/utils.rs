use ahash::{HashMap, HashMapExt};
use smallvec::smallvec;

use crate::{Dummy, Dummy::EdgeProxy, Graph, GraphNode, Key, Point};

static mut KEY_COUNTER: Key = 2;

#[inline]
pub(crate) fn unique_key() -> Key {
    unsafe {
        KEY_COUNTER += 1;
        KEY_COUNTER
    }
}

impl Graph {
    fn max_rank(&self) -> usize {
        self.nodes.values().map(|n| n.rank.unwrap()).max().unwrap_or(0) as usize
    }

    pub(super) fn add_dummy_node(&mut self, dummy: Dummy, mut node: GraphNode) -> Key {
        let mut node_id = unique_key();
        while self.has_node(&node_id) {
            node_id = unique_key();
        }

        node.dummy = Some(dummy);
        self.set_node(node_id, Some(node));
        node_id
    }

    pub(super) fn as_non_compound(&mut self) -> Graph {
        let mut simplified: Graph = Graph::new(true, false);
        simplified.config = self.config;

        self.transfer_node_edges(&mut simplified);

        simplified
    }

    pub(super) fn transfer_node_edges(&mut self, dst: &mut Graph) {
        for (&node_id, &node) in &self.nodes {
            if self.children(&node_id).is_empty() {
                dst.set_node(node_id, Some(node));
            }
        }

        for &edge in self.edges.values() {
            dst.set_edge1(edge, self.edge1(edge).cloned());
        }
    }

    /// Adjusts rank for all nodes, then all node sources have
    /// **rank(source) >= 0**
    /// and at least one node target has **rank(target) = 0**.
    pub(super) fn normalize_ranks(&mut self) {
        let min = self.nodes.values().map(|n| n.rank.unwrap_or(0)).min().unwrap_or(0);

        for node in &mut self.nodes.values_mut() {
            if let Some(rank) = node.rank {
                node.rank = Some(rank - min)
            }
        }
    }

    pub(super) fn remove_empty_ranks(&mut self) {
        /// Ranks may not start at 0, so we need to offset them
        let offset = self.nodes.values().map(|n| n.rank.unwrap_or(0)).min().unwrap_or(0);

        let mut layers: HashMap<i32, Vec<Key>> = HashMap::new();
        for (&node_id, node) in &self.nodes {
            let rank = node.rank.unwrap_or(0).wrapping_sub(offset);
            layers.entry(rank).or_default().push(node_id)
        }

        let mut delta = 0;
        let node_rank_factor = self.config.node_rank_factor as i32;
        for (rank, keys) in &layers {
            if keys.is_empty() && rank % node_rank_factor != 0 {
                delta -= 1;
            } else if delta != 0 {
                for k in keys {
                    self.node_mut(k).map(|n| n.rank = Some(n.rank.unwrap_or(0) + delta));
                }
            }
        }
    }

    /// Given a DAG with each node assigned "rank" and "order" properties, this
    /// function produces a matrix with the keys of each node.
    pub(super) fn key_matrix(&self) -> Vec<Vec<Key>> {
        let mut matrix: Vec<Vec<(usize, Key)>> = vec![Vec::new(); self.max_rank() + 1];

        for (&key, node) in &self.nodes {
            if let Some(rank) = node.rank {
                matrix[rank as usize].push((node.order.unwrap(), key));
            }
        }

        matrix
            .iter_mut()
            .map(|layer| {
                layer.sort_by_key(|&(order, _)| order);
                layer.iter().map(|&(_, key)| key).collect()
            })
            .collect()
    }

    pub(super) fn make_space_for_edge_labels(&mut self) {
        let graph_config = &mut self.config;
        graph_config.ranksep = graph_config.ranksep / 2.0;

        for edge in self.edge_values.values_mut() {
            let minlen = edge.minlen.unwrap_or(1);
            edge.minlen = Some(minlen * 2);
        }
    }

    pub(super) fn assign_rank_min_max(&mut self) -> Option<()> {
        for key in self.nodes().iter() {
            let node = self.node(key)?;
            if let (Some(border_top), Some(border_bottom)) = (&node.border_top, &node.border_bottom)
            {
                let min_rank = self.node(border_top).and_then(|n| n.rank).unwrap_or(0);
                let max_rank = self.node(border_bottom).and_then(|n| n.rank).unwrap_or(0);

                let node = self.node_mut(key)?;
                node.min_rank = Some(min_rank);
                node.max_rank = Some(max_rank);
            }
        }

        None
    }

    pub(super) fn translate_graph(&mut self) {
        let mut min_x = f64::INFINITY as f32;
        let mut max_x: f32 = 0.0;
        let mut min_y = f64::INFINITY as f32;
        let mut max_y: f32 = 0.0;

        for GraphNode { x, y, width, height, .. } in self.nodes.values() {
            min_x = min_x.min(x - width / 2.0);
            max_x = max_x.max(x + width / 2.0);
            min_y = min_y.min(y - height / 2.0);
            max_y = max_y.max(y + height / 2.0);
        }

        for node in self.nodes.values_mut() {
            node.x -= min_x;
            node.y -= min_y;
        }

        for edge in self.edge_values.values_mut() {
            if let Some(points) = &mut edge.points {
                for point in points {
                    point.x -= min_x;
                    point.y -= min_y;
                }
            }
        }

        self.width = max_x - min_x;
        self.height = max_y - min_y;
    }

    pub(super) fn assign_node_intersects(&mut self) -> Option<()> {
        for e in self.edges() {
            let ((source_p,source_bbox), (target_p, target_bbox)) = {
                let source_node = self.node(&e.source)?;
                let target_node = self.node(&e.target)?;
                (source_node.coord_bbox(), target_node.coord_bbox())
            };
            
            let edge = self.edge_mut1(e)?;

            if let Some(points) = &mut edge.points {
                let p1 = source_bbox.intersect_point(Point::of(points[0].x, points[0].y));
                points.insert(0, p1);
                let p2 = target_bbox.intersect_point(Point::of(
                    points[points.len() - 1].x,
                    points[points.len() - 1].y,
                ));
                points.push(p2);
            } else {
                let p1 = source_bbox.intersect_point(target_p);
                let p2 = target_bbox.intersect_point(source_p);
                edge.points = Some(smallvec![p1, p2]);
            };
        }

        None
    }

    pub(super) fn remove_edge_proxies(&mut self) -> Option<()> {
        for key in &self.nodes() {
            let node = self.node(key)?;
            if node.dummy == Some(EdgeProxy) {
                let rank = node.rank.unwrap_or(0);
                if let Some(graph_edge) = self.edge_mut1(node.edge?) {
                    graph_edge.rank = Some(rank);
                }
                self.remove_node(key);
            }
        }

        None
    }

    pub(super) fn reverse_points_for_reversed_edges(&mut self) {
        for edge in self.edge_values.values_mut() {
            if edge.reversed
                && let Some(points) = &mut edge.points
            {
                points.reverse();
            }
        }
    }
}

#[derive(Copy, Clone)]
pub(crate) struct Rect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

impl GraphNode {
    #[inline]
    fn bbox(&self) -> Rect {
        Rect { x: self.x, y: self.y, width: self.width, height: self.height }
    }

    #[inline]
    fn coord_bbox(&self) -> (Point, Rect) {
        (Point::of(self.x, self.y), self.bbox())
    }
}

impl Rect {
    /// Finds where a line starting at point {x, y} would intersect a rectangle
    /// {x, y, width, height} if it were pointing at the rectangle's center.
    pub(crate) fn intersect_point(self, point: Point) -> Point {
        let x = self.x;
        let y = self.y;

        /// Rectangle intersection algorithm
        /// [math.stackoverflow](http://math.stackexchange.com/questions/108113/find-edge-between-two-boxes):
        let dx = point.x - x;
        let dy = point.y - y;
        let w = self.width / 2.0;
        let h = self.height / 2.0;

        let (sx, sy) = if (dy.abs() * w) > (dx.abs() * h) {
            if dy < 0.0 { (-h * dx / dy, -h) } else { (h * dx / dy, h) }
        } else {
            if dx < 0.0 { (-w, -w * dy / dx) } else { (w, w * dy / dx) }
        };

        Point::of(x + sx, y + sy)
    }
}
