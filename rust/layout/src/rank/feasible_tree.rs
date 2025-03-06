use crate::{Edge, Graph, GraphEdge, GraphNode, Key, EMPTY_KEY};

impl Graph {
    pub(super) fn feasible_tree(&mut self) -> Graph {
        let mut t: Graph = Graph::new(false, false);

        let start = self.nodes().first().copied().unwrap_or(EMPTY_KEY);
        let size = self.nodes.len();
        t.set_node(start, Some(GraphNode::default()));

        while tight_tree(&mut t, self) < size {
            if let Some(edge) = find_min_stack_edge(&t, self) {
                let delta =
                    if t.has_node(&edge.source) { self.slack(edge) } else { -self.slack(edge) };
                shift_ranks(&t, self, delta);
            }
        }

        t
    }
}

fn tight_tree(t: &mut Graph, g: &Graph) -> usize {
    let mut stack: Vec<Key> = t.nodes();

    while let Some(curr) = stack.pop() {
        for edge in g.node_edges(&curr) {
            let source = edge.source;
            let key = if curr == source { edge.target } else { edge.source };
            if !t.has_node(&key) && g.slack(edge) == 0 {
                t.set_node(key, Some(GraphNode::default()));
                t.set_edge_undirected(curr, key, Some(GraphEdge::default()));
                stack.push(key);
            }
        }
    }

    t.nodes.len()
}

fn find_min_stack_edge(t: &Graph, g: &Graph) -> Option<Edge> {
    g.edges
        .values()
        .filter_map(|&e| (t.has_node(&e.source) != t.has_node(&e.target)).then(|| (e, g.slack(e))))
        .min_by_key(|(_, slack)| *slack)
        .map(|(e, _)| e)
}

fn shift_ranks(t: &Graph, g: &mut Graph, delta: i32) {
    for node_id in t.nodes.keys() {
        if let Some(node) = g.node_mut(node_id) {
            node.rank = Some(node.rank.unwrap_or(0).wrapping_add(delta));
        }
    }
}
