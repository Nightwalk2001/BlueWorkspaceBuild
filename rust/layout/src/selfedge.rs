use smallvec::smallvec;

use crate::{Dummy::SelfEdge, Graph, GraphNode, Point};

/// Self-edge Processing Module
///
/// ## Overview
/// This module implements a three-phase process to layout 
/// self-edges properly in hierarchical graph diagrams.
/// Self-edge is edge where source == target
///
/// The implementation handles:
/// - Temporary removal of original edges
/// - Insertion of layout markers
/// - Final path calculation and cleanup
///
/// ## Operation Phases
/// 1. **Edge Removal** (`remove_self_edges`):
///    - Detects and removes self-edges from the main graph structure
///    - Stores original edges in a temporary map for later processing
///
/// 2. **Marker Insertion** (`insert_self_edges`):
///    - Creates dummy nodes in the layout matrix to reserve space
///    - Maintains proper node ordering through order shifting
///    - Preserves edge metadata for final rendering
///
/// 3. **Path Positioning** (`position_self_edges`):
///    - Calculates smooth Bézier curve control points
///    - Creates an elliptical path around source node
///    - Removes temporary dummy nodes after path generation
///
/// ## Key Characteristics
/// - Maintains layout integrity through temporary dummy nodes
/// - Generates consistent elliptical paths for visual clarity
/// - Preserves original-edge data while modifying visual representation
impl Graph {
    pub(super) fn remove_self_edges(&mut self) {
        for edge in self.edges() {
            if edge.source == edge.target {
                self.selfedge_map.entry(edge.source).or_default().push(edge);
                self.remove_edge1(edge);
            }
        }
    }

    pub(super) fn insert_self_edges(&mut self) -> Option<()> {
        let matrix = self.key_matrix();
        for layer in matrix {
            let mut order_shift = 0;
            for (i, id) in layer.iter().enumerate() {
                let node = self.node_mut(id)?;
                node.order = Some(i + order_shift);
                let rank = node.rank;

                let self_edges = self.selfedge_map.get(id)?.clone();
                for edge in self_edges {
                    order_shift += 1;
                    let graph_node = GraphNode {
                        rank,
                        order: Some(i + order_shift),
                        edge: Some(edge),
                        ..GraphNode::default()
                    };
                    self.add_dummy_node(SelfEdge, graph_node);
                }
            }
        }

        None
    }

    pub(super) fn position_self_edges(&mut self) -> Option<()> {
        for key in &mut self.nodes() {
            let node = &mut self.node(&key)?;
            if node.dummy == Some(SelfEdge) {
                let self_node = &mut self.node(&node.edge?.source)?;
                let x = self_node.x + self_node.width / 2.0;
                let y = self_node.y;
                let dx = node.x - x;
                let dy = self_node.height / 2.0;
                let graph_edge = &mut self.edge_mut1(node.edge?)?;
                graph_edge.points = Some(smallvec![
                    Point::of(x + 2.0 * dx / 3.0, y - dy),
                    Point::of(x + 2.0 * dx / 3.0, y - dy),
                    Point::of(x + 5.0 * dx / 6.0, y - dy),
                    Point::of(x + dx, y),
                    Point::of(x + 5.0 * dx / 6.0, y + dy),
                    Point::of(x + 2.0 * dx / 3.0, y + dy),
                ]);
                self.remove_node(&key);
            }
        }

        None
    }
}
